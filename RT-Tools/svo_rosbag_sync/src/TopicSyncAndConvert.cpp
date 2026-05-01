#include "TopicSyncAndConvert.hpp"

template <class T_in, class T_out>
TopicSyncAndConvert<T_in, T_out>::TopicSyncAndConvert(
  std::string const & topic, SvoRosbagSyncNode * node, std::function<T_out(T_in)> converter)
{
  _topic = topic;
  _converter = converter;
  _publisher = node->create_publisher<T_out>(topic, 10);
}

template <class T_in, class T_out>
bool TopicSyncAndConvert<T_in, T_out>::read_message(
  rosbag2_storage::SerializedBagMessageSharedPtr msg)
{
  if (msg->topic_name != _topic) {
    return false;
  }

  rclcpp::SerializedMessage serialized_msg(*msg->serialized_data);
  _ros_msg = std::make_shared<T_in>();
  _ros_msg_time_stamp = msg->time_stamp;

  _serializer.deserialize_message(&serialized_msg, _ros_msg.get());
  _message_available = true;

  return true;
}

template <class T_in, class T_out>
bool TopicSyncAndConvert<T_in, T_out>::timer_callback(
  uint64_t zed_timestamp, uint64_t now, const rclcpp::Node * node)
{
  if (!_message_available) {
    return false;
  }

  if (_ros_msg_time_stamp > now - zed_timestamp) {
    return false;
  }

  T_out converted_ros_msg = _converter(*_ros_msg);

  converted_ros_msg.header.stamp.sec = now / 1000000000ul;
  converted_ros_msg.header.stamp.nanosec = now % 1000000000ul;

  _publisher->publish(converted_ros_msg);
  _message_available = false;
  RCLCPP_DEBUG(node->get_logger(), "publishing to %s\n", _topic.c_str());
  return true;
}