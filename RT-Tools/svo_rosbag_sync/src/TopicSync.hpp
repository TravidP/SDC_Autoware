#pragma once
#include <chrono>

#include "SvoRosbagSyncNode.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/serialization.hpp"
#include "rosbag2_cpp/reader.hpp"

class SvoRosbagSyncNode;  // remove compiler error caused by circular dependency

template <typename T>
class TopicSync
{
public:
  TopicSync(std::string const & topic, SvoRosbagSyncNode * node);

  bool read_message(rosbag2_storage::SerializedBagMessageSharedPtr msg);

  bool timer_callback(uint64_t zed_timestamp, uint64_t now, const rclcpp::Node * node);

private:
  std::string _topic;
  bool _message_available = false;

  typename rclcpp::Publisher<T>::SharedPtr _publisher;
  typename rclcpp::Serialization<T> _serializer;

  typename T::SharedPtr _ros_msg;
  uint64_t _ros_msg_time_stamp;
};