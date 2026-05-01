#include "TfTopicSync.hpp"

TfTopicSync::TfTopicSync(std::string const & topic, SvoRosbagSyncNode * node)
{
  _topic = topic;
  _tf_broadcaster = std::make_unique<tf2_ros::TransformBroadcaster>(node);
}

bool TfTopicSync::read_message(rosbag2_storage::SerializedBagMessageSharedPtr msg)
{
  if (msg->topic_name != _topic) {
    return false;
  }

  rclcpp::SerializedMessage serialized_msg(*msg->serialized_data);
  _ros_msg = std::make_shared<tf2_msgs::msg::TFMessage>();

  _serializer.deserialize_message(&serialized_msg, _ros_msg.get());
  _message_available = true;

  return true;
}

bool TfTopicSync::timer_callback(uint64_t zed_timestamp, uint64_t now, rclcpp::Node * node)
{
  if (!_message_available) {
    return false;
  }

  uint64_t first_header_timestamp = _ros_msg->transforms[0].header.stamp.sec * 1000000000ul +
                                    _ros_msg->transforms[0].header.stamp.nanosec;
  if (first_header_timestamp > now - zed_timestamp) {
    return false;
  }

  // TF tree for fixposition:
  //
  //             --- FP_ECEF ---
  //            |               |
  //        --- FP_POI --    FP_ENU0
  //       |      |      |
  // FP_VRTK   FP_IMUH   FP_POISH
  //    |
  // FP_CAM
  //
  // (source: https://docs.fixposition.com/fd/fixposition-ros-driver)

  for (auto & t_old : _ros_msg->transforms) {
    if (
      t_old.header.frame_id != "FP_ECEF" && t_old.header.frame_id != "FP_POI" &&
      t_old.header.frame_id != "FP_VRTK") {
      continue;
    }

    if (
      t_old.child_frame_id != "FP_POI" && t_old.child_frame_id != "FP_ENU0" &&
      t_old.child_frame_id != "FP_VRTK" && t_old.child_frame_id != "FP_IMUH" &&
      t_old.child_frame_id != "FP_POISH" && t_old.child_frame_id != "FP_CAM") {
      continue;
    }

    geometry_msgs::msg::TransformStamped t_new;
    t_new.header = t_old.header;
    t_new.child_frame_id = t_old.child_frame_id;
    t_new.transform = t_old.transform;

    t_new.header.stamp = node->get_clock()->now();

    _tf_broadcaster->sendTransform(t_new);
    RCLCPP_DEBUG(
      node->get_logger(), "publishing to %s: %s -> %s\n", _topic.c_str(),
      t_new.header.frame_id.c_str(), t_new.child_frame_id.c_str());
  }

  _message_available = false;
  return true;
}