#include "TopicSync.hpp"

template <class T>
TopicSync<T>::TopicSync(std::string const & topic, SvoRosbagSyncNode * node)
{
  _topic = topic;
  _publisher = node->create_publisher<T>(topic, 10);
}

template <class T>
bool TopicSync<T>::read_message(rosbag2_storage::SerializedBagMessageSharedPtr msg)
{
  if (msg->topic_name != _topic) {
    return false;
  }

  rclcpp::SerializedMessage serialized_msg(*msg->serialized_data);
  _ros_msg = std::make_shared<T>();
  _ros_msg_time_stamp = msg->time_stamp;

  _serializer.deserialize_message(&serialized_msg, _ros_msg.get());
  _message_available = true;

  return true;
}

template <class T>
bool TopicSync<T>::timer_callback(uint64_t zed_timestamp, uint64_t now, const rclcpp::Node * node)
{
  if (!_message_available) {
    return false;
  }

  if (_ros_msg_time_stamp > now - zed_timestamp) {
    return false;
  }

  _ros_msg->header.stamp.sec = now / 1000000000ul;
  _ros_msg->header.stamp.nanosec = now % 1000000000ul;

  _publisher->publish(*_ros_msg);
  _message_available = false;
  RCLCPP_DEBUG(node->get_logger(), "publishing to %s\n", _topic.c_str());
  return true;
}