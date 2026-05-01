#include <GeographicLib/Geocentric.hpp>
#include <cmath>

#include "fixposition_driver_msgs/msg/fpa_llh.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2/exceptions.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"

class LLHTransformer : public rclcpp::Node
{
public:
  LLHTransformer() : Node("llh_transformer")
  {
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    rclcpp::QoS qos_profile(1);                                       // Depth set to 1
    qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);  // Best Effort reliability
    qos_profile.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);       // Volatile durability
    qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);            // Keep last history

    gnss_sub_ = this->create_subscription<fixposition_driver_msgs::msg::FpaLlh>(
      "/fixposition/fpa/llh", qos_profile,
      std::bind(&LLHTransformer::llh_callback, this, std::placeholders::_1));

    ecef_to_base_link_odom_pub_ =
      this->create_publisher<nav_msgs::msg::Odometry>("/ecef_to_base_link/odometry", 10);
    transformed_gnss_pub_ = this->create_publisher<fixposition_driver_msgs::msg::FpaLlh>(
      "/fixposition/fpa/llh_trans", 10);
  }

private:
  void llh_callback(const fixposition_driver_msgs::msg::FpaLlh::SharedPtr msg)
  {
    geometry_msgs::msg::TransformStamped transform;
    geometry_msgs::msg::TransformStamped transform_enu0;
    try {
      transform = tf_buffer_->lookupTransform("FP_ECEF", "base_link", rclcpp::Time(0));
    } catch (tf2::TransformException & ex) {
      RCLCPP_ERROR(this->get_logger(), "Transform error: %s", ex.what());
      return;
    }

    // Extract the ECEF coordinates
    double ecef_x = transform.transform.translation.x;
    double ecef_y = transform.transform.translation.y;
    double ecef_z = transform.transform.translation.z;

    double latitude, longitude, altitude;
    GeographicLib::Geocentric earth(
      GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f());
    earth.Reverse(ecef_x, ecef_y, ecef_z, latitude, longitude, altitude);

    fixposition_driver_msgs::msg::FpaLlh transformed_gnss_msg;
    transformed_gnss_msg.header.stamp = msg->header.stamp;
    transformed_gnss_msg.position.x = latitude;
    transformed_gnss_msg.position.y = longitude;
    transformed_gnss_msg.position.z = altitude;

    nav_msgs::msg::Odometry ecef_to_baselink_msg;
    ecef_to_baselink_msg.header.stamp = msg->header.stamp;
    ecef_to_baselink_msg.pose.pose.position.x = ecef_x;
    ecef_to_baselink_msg.pose.pose.position.y = ecef_y;
    ecef_to_baselink_msg.pose.pose.position.z = ecef_z;
    ecef_to_baselink_msg.pose.pose.orientation.x = transform.transform.rotation.x;
    ecef_to_baselink_msg.pose.pose.orientation.y = transform.transform.rotation.y;
    ecef_to_baselink_msg.pose.pose.orientation.z = transform.transform.rotation.z;
    ecef_to_baselink_msg.pose.pose.orientation.w = transform.transform.rotation.w;

    transformed_gnss_pub_->publish(transformed_gnss_msg);
    ecef_to_base_link_odom_pub_->publish(ecef_to_baselink_msg);
  }

  rclcpp::Subscription<fixposition_driver_msgs::msg::FpaLlh>::SharedPtr gnss_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr ecef_to_base_link_odom_pub_;
  rclcpp::Publisher<fixposition_driver_msgs::msg::FpaLlh>::SharedPtr transformed_gnss_pub_;

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LLHTransformer>());
  rclcpp::shutdown();
  return 0;
}
