#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <exception>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <memory>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>

using namespace std::chrono_literals;
using std::placeholders::_1;

class GDKinematicStatePublisher : public rclcpp::Node {
 public:
  GDKinematicStatePublisher() : Node("GDKinematicStatePublisher") {
    rate_ = declare_parameter<int>("rate");
    kinematic_state_topic_ =
        declare_parameter<std::string>("kinematic_state_topic");
    fixpostion_odometry_topic_ =
        declare_parameter<std::string>("fixpostion_odometry_topic");
    source_frame_ = declare_parameter<std::string>("source_frame");
    target_frame_ = declare_parameter<std::string>("target_frame");
    tf_timeout_ = declare_parameter<int>("tf_timeout");

    kinematic_state_pub_ = this->create_publisher<nav_msgs::msg::Odometry>(
        kinematic_state_topic_, 10);
    rclcpp::QoS qos_profile(1);  // Depth set to 1
    qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);  // Best Effort reliability
    qos_profile.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);  // Volatile durability
    qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);  // Keep last history
    fixposition_odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
        fixpostion_odometry_topic_, qos_profile,
        std::bind(&GDKinematicStatePublisher::odometry_callback, this, _1));

    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
    timer_ = create_wall_timer(
        std::chrono::milliseconds(1000 / rate_),
        std::bind(&GDKinematicStatePublisher::timer_callback, this));

    FP_POI_to_base_link_transform_.setIdentity();
    try {
      std::string source = "FP_POI";
      geometry_msgs::msg::TransformStamped FP_POI_to_base_link_tf_msg =
          tf_buffer_->lookupTransform(source, target_frame_, rclcpp::Time(0),
                                      rclcpp::Duration(tf_timeout_, 0));
      FP_POI_to_base_link_transform_.setRotation(
          tf2::Quaternion(FP_POI_to_base_link_tf_msg.transform.rotation.x,
                          FP_POI_to_base_link_tf_msg.transform.rotation.y,
                          FP_POI_to_base_link_tf_msg.transform.rotation.z,
                          FP_POI_to_base_link_tf_msg.transform.rotation.w));
      FP_POI_to_base_link_transform_.setOrigin(
          tf2::Vector3(FP_POI_to_base_link_tf_msg.transform.translation.x,
                       FP_POI_to_base_link_tf_msg.transform.translation.y,
                       FP_POI_to_base_link_tf_msg.transform.translation.z));
    } catch (const tf2::TransformException& ex) {
      RCLCPP_ERROR_STREAM(this->get_logger(),
                          "Transform from FP_POI"
                              << " to " << target_frame_
                              << " not found. Exception: " << ex.what());
      std::terminate();
    }

    linear_velocity_.setZero();
    angular_velocity_.setZero();
  }

 private:
  void timer_callback() {
    geometry_msgs::msg::TransformStamped map2base_link_transform_msg;
    try {
      map2base_link_transform_msg = tf_buffer_->lookupTransform(
          source_frame_, target_frame_, tf2::TimePointZero);
    } catch (tf2::TransformException& ex) {
      RCLCPP_ERROR(get_logger(), "Transform exception: %s", ex.what());
      return;
    }
    nav_msgs::msg::Odometry msg;
    msg.header.frame_id = source_frame_;
    msg.child_frame_id = target_frame_;
    msg.header.stamp = map2base_link_transform_msg.header.stamp;

    // TODO There is probably a ros2 function that performs the following
    // conversion
    msg.pose.pose.position.x =
        map2base_link_transform_msg.transform.translation.x;
    msg.pose.pose.position.y =
        map2base_link_transform_msg.transform.translation.y;
    msg.pose.pose.position.z =
        map2base_link_transform_msg.transform.translation.z;
    msg.pose.pose.orientation.x =
        map2base_link_transform_msg.transform.rotation.x;
    msg.pose.pose.orientation.y =
        map2base_link_transform_msg.transform.rotation.y;
    msg.pose.pose.orientation.z =
        map2base_link_transform_msg.transform.rotation.z;
    msg.pose.pose.orientation.w =
        map2base_link_transform_msg.transform.rotation.w;

    //Transforming the linear/angular velocities from FP_POI to base_link
    tf2::Vector3 linear_vel =
        FP_POI_to_base_link_transform_.getBasis() *
        (linear_velocity_ +
         angular_velocity_.cross(FP_POI_to_base_link_transform_.getOrigin()));
    tf2::Vector3 angular_vel =
        FP_POI_to_base_link_transform_.getBasis() * angular_velocity_;
    
    msg.twist.twist.linear.x = linear_vel.getX();
    msg.twist.twist.linear.y = linear_vel.getY();
    msg.twist.twist.linear.z = linear_vel.getZ();
    msg.twist.twist.angular.x = angular_vel.getX();
    msg.twist.twist.angular.y = angular_vel.getY();
    msg.twist.twist.angular.z = angular_vel.getZ();

    kinematic_state_pub_->publish(msg);
    return;
  }

  void odometry_callback(const nav_msgs::msg::Odometry::ConstPtr msg) {
    linear_velocity_.setX(msg->twist.twist.linear.x);
    linear_velocity_.setY(msg->twist.twist.linear.y);
    linear_velocity_.setZ(msg->twist.twist.linear.z);
    angular_velocity_.setX(msg->twist.twist.angular.x);
    angular_velocity_.setY(msg->twist.twist.angular.y);
    angular_velocity_.setZ(msg->twist.twist.angular.z);
    return;
  }
    
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr kinematic_state_pub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr
      autoware_localization_state_pub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr
      fixposition_odom_sub_;
  rclcpp::TimerBase::SharedPtr timer_;
  tf2::Transform FP_POI_to_base_link_transform_;
  tf2::Vector3 linear_velocity_;
  tf2::Vector3 angular_velocity_;

  /*Parameters*/
  int rate_;
  int tf_timeout_;
  std::string kinematic_state_topic_;
  std::string fixpostion_odometry_topic_;
  std::string source_frame_;
  std::string target_frame_;
};
