#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>

class OdomToTFPublisher : public rclcpp::Node
{
public:
  OdomToTFPublisher() : Node("odom_to_tf_publisher")
  {
    // Declare parameters
    this->declare_parameter("parent_frame_id", "map");
    this->declare_parameter("child_frame_id", "base_link");
    this->declare_parameter("override_frame_id", false);
    this->declare_parameter("qos_reliability", "best_effort");
    this->declare_parameter("qos_durability", "volatile");
    this->declare_parameter("qos_history_depth", 10);
    
    parent_frame_id_ = this->get_parameter("parent_frame_id").as_string();
    child_frame_id_ = this->get_parameter("child_frame_id").as_string();
    override_frame_id_ = this->get_parameter("override_frame_id").as_bool();
    
    std::string qos_reliability = this->get_parameter("qos_reliability").as_string();
    std::string qos_durability = this->get_parameter("qos_durability").as_string();
    int qos_history_depth = this->get_parameter("qos_history_depth").as_int();
    
    // Create TF broadcaster
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    
    // Configure QoS
    rclcpp::QoS qos(qos_history_depth);
    
    if (qos_reliability == "reliable") {
      qos.reliable();
    } else if (qos_reliability == "best_effort") {
      qos.best_effort();
    } else {
      RCLCPP_WARN(this->get_logger(), 
                  "Unknown reliability '%s', defaulting to best_effort", 
                  qos_reliability.c_str());
      qos.best_effort();
    }
    
    if (qos_durability == "transient_local") {
      qos.transient_local();
    } else if (qos_durability == "volatile") {
      qos.durability_volatile();
    } else {
      RCLCPP_WARN(this->get_logger(), 
                  "Unknown durability '%s', defaulting to volatile", 
                  qos_durability.c_str());
      qos.durability_volatile();
    }
    
    // Subscribe to Odometry topic with configured QoS
    odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "odom", qos,
      std::bind(&OdomToTFPublisher::odomCallback, this, std::placeholders::_1));
    
    RCLCPP_INFO(this->get_logger(), 
                "OdomToTF Publisher started with QoS: reliability=%s, durability=%s, depth=%d", 
                qos_reliability.c_str(), qos_durability.c_str(), qos_history_depth);
    
    if (override_frame_id_) {
      RCLCPP_INFO(this->get_logger(), 
                  "Publishing TF: %s -> %s (overriding input frame_id)", 
                  parent_frame_id_.c_str(), child_frame_id_.c_str());
    } else {
      RCLCPP_INFO(this->get_logger(), 
                  "Publishing TF: <from_message> -> %s", 
                  child_frame_id_.c_str());
    }
  }

private:
  void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    geometry_msgs::msg::TransformStamped transform;
    
    // Set header
    transform.header.stamp = msg->header.stamp;
    
    // Use parameter for parent frame or message frame_id
    if (override_frame_id_) {
      transform.header.frame_id = parent_frame_id_;
    } else {
      transform.header.frame_id = msg->header.frame_id;
    }
    
    transform.child_frame_id = child_frame_id_;
    
    // Copy position from odometry
    transform.transform.translation.x = msg->pose.pose.position.x;
    transform.transform.translation.y = msg->pose.pose.position.y;
    transform.transform.translation.z = msg->pose.pose.position.z;
    
    // Copy orientation from odometry
    transform.transform.rotation = msg->pose.pose.orientation;
    
    // Broadcast the transform
    tf_broadcaster_->sendTransform(transform);
  }
  
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  std::string parent_frame_id_;
  std::string child_frame_id_;
  bool override_frame_id_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<OdomToTFPublisher>());
  rclcpp::shutdown();
  return 0;
}