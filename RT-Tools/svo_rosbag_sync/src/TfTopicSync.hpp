#pragma once
#include <chrono>

#include "SvoRosbagSyncNode.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/serialization.hpp"
#include "rosbag2_cpp/reader.hpp"
#include "tf2_msgs/msg/tf_message.hpp"
#include "tf2_ros/transform_broadcaster.h"

class SvoRosbagSyncNode;  // remove compiler error caused by circular dependency

class TfTopicSync
{
public:
  TfTopicSync(std::string const & topic, SvoRosbagSyncNode * node);

  bool read_message(rosbag2_storage::SerializedBagMessageSharedPtr msg);

  bool timer_callback(uint64_t zed_timestamp, uint64_t now, rclcpp::Node * node);

private:
  std::string _topic;
  bool _message_available = false;

  std::unique_ptr<tf2_ros::TransformBroadcaster> _tf_broadcaster;
  rclcpp::Serialization<tf2_msgs::msg::TFMessage> _serializer;

  typename tf2_msgs::msg::TFMessage::SharedPtr _ros_msg;
};