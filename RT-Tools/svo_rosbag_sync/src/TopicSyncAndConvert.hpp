#pragma once

#include <chrono>

#include "SvoRosbagSyncNode.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/serialization.hpp"
#include "rosbag2_cpp/reader.hpp"

class SvoRosbagSyncNode;  // remove compiler error caused by circular dependency

template <typename T_in, typename T_out>
class TopicSyncAndConvert
{
public:
  TopicSyncAndConvert(
    std::string const & topic, SvoRosbagSyncNode * node, std::function<T_out(T_in)> converter);

  bool read_message(rosbag2_storage::SerializedBagMessageSharedPtr msg);

  bool timer_callback(uint64_t zed_timestamp, uint64_t now, const rclcpp::Node * node);

private:
  std::string _topic;
  bool _message_available = false;

  std::function<T_out(T_in)> _converter;

  typename rclcpp::Publisher<T_out>::SharedPtr _publisher;
  typename rclcpp::Serialization<T_in> _serializer;

  typename T_in::SharedPtr _ros_msg;
  uint64_t _ros_msg_time_stamp;
};