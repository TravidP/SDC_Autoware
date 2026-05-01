#include "SvoRosbagSyncNode.hpp"

#include <chrono>
#include <fstream>
#include <string>

using namespace std::chrono_literals;
using std::placeholders::_1;

// autoware_control_msgs::msg::Control
// convertControl(autoware_auto_control_msgs::msg::AckermannControlCommand in) {
//     autoware_control_msgs::msg::Control out;
//     out.stamp = in.stamp;
//     out.lateral.stamp = in.lateral.stamp;
//     out.lateral.steering_tire_angle = in.lateral.steering_tire_angle;
//     out.lateral.steering_tire_rotation_rate =
//     in.lateral.steering_tire_rotation_rate; out.longitudinal.stamp =
//     in.longitudinal.stamp; out.longitudinal.velocity = in.longitudinal.speed;
//     out.longitudinal.acceleration = in.longitudinal.acceleration;
//     out.longitudinal.jerk = in.longitudinal.jerk;
//     return out;
// }

SvoRosbagSyncNode::SvoRosbagSyncNode() : Node("svo_rosbag_sync_node")
{
  this->declare_parameter("bag_filename", "");
  this->declare_parameter("timestamps_filename", "");

  std::string bag_filename = this->get_parameter("bag_filename").as_string();
  std::string timestamps_filename = this->get_parameter("timestamps_filename").as_string();

  load_zed_timestamps(timestamps_filename);

  _diagnostics_subscription = this->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
    "/diagnostics", 10, std::bind(&SvoRosbagSyncNode::diagnostics_callback, this, _1));

  _fpa_llh_sync =
    std::make_shared<TopicSync<fixposition_driver_ros2::msg::LLH>>("/fixposition/fpa/llh", this);
  _fpa_llh_trans_sync = std::make_shared<TopicSync<fixposition_driver_ros2::msg::LLH>>(
    "/fixposition/fpa/llh_trans", this);
  _fpa_odometry_sync = std::make_shared<TopicSync<fixposition_driver_ros2::msg::ODOMETRY>>(
    "/fixposition/fpa/odometry", this);
  _fpa_odomsh_sync = std::make_shared<TopicSync<fixposition_driver_ros2::msg::ODOMSH>>(
    "/fixposition/fpa/odomsh", this);
  _odometry_ecef_sync =
    std::make_shared<TopicSync<nav_msgs::msg::Odometry>>("/fixposition/odometry_ecef", this);
  _odometry_llh_sync =
    std::make_shared<TopicSync<sensor_msgs::msg::NavSatFix>>("/fixposition/odometry_llh", this);
  _poiimu_sync = std::make_shared<TopicSync<sensor_msgs::msg::Imu>>("/fixposition/poiimu", this);
  _rawimu_sync = std::make_shared<TopicSync<sensor_msgs::msg::Imu>>("/fixposition/rawimu", this);

  _velocity_status_sync = std::make_shared<TopicSync<autoware_vehicle_msgs::msg::VelocityReport>>(
    "/vehicle/status/velocity_status", this);
  _steering_status_sync = std::make_shared<TopicSync<autoware_vehicle_msgs::msg::SteeringReport>>(
    "/vehicle/status/steering_status", this);
  _predicted_trajectory_sync = std::make_shared<TopicSync<autoware_planning_msgs::msg::Trajectory>>(
    "/control/trajectory_follower/lateral/predicted_trajectory", this);
  // _control_command_sync =
  // std::make_shared<TopicSyncAndConvert<autoware_auto_control_msgs::msg::AckermannControlCommand,
  // autoware_control_msgs::msg::Control>>(
  //         "/control/command/control_cmd", this,
  //         convertControl);

  _continental_objects_sync =
    std::make_shared<TopicSync<continental_msgs::msg::ContinentalArs548ObjectList>>(
      "/continental_objects", this);

  _tf_sync = std::make_shared<TfTopicSync>("/tf", this);
  _tf_static_sync = std::make_shared<TfTopicSync>("/tf_static", this);

  _timer = this->create_wall_timer(1ms, std::bind(&SvoRosbagSyncNode::timer_callback, this));

  _reader.open(bag_filename);
  read_message();
}

void SvoRosbagSyncNode::load_zed_timestamps(const std::string & timestamps_filename)
{
  std::ifstream file_stream(timestamps_filename);
  if (file_stream.is_open()) {
    std::string line;
    while (getline(file_stream, line)) {
      _zed_timestamps.push_back(stoll(line));
    }
    file_stream.close();
  }
  RCLCPP_INFO(get_logger(), "zed timestamps loaded: %zu\n", _zed_timestamps.size());
}

void SvoRosbagSyncNode::read_message()
{
  while (_reader.has_next()) {
    rosbag2_storage::SerializedBagMessageSharedPtr msg = _reader.read_next();

    bool read_success = false;
    read_success |= _fpa_llh_sync->read_message(msg);
    read_success |= _fpa_llh_trans_sync->read_message(msg);
    read_success |= _fpa_odometry_sync->read_message(msg);
    read_success |= _fpa_odomsh_sync->read_message(msg);
    read_success |= _odometry_ecef_sync->read_message(msg);
    read_success |= _odometry_llh_sync->read_message(msg);
    read_success |= _poiimu_sync->read_message(msg);
    read_success |= _rawimu_sync->read_message(msg);

    read_success |= _velocity_status_sync->read_message(msg);
    read_success |= _steering_status_sync->read_message(msg);
    read_success |= _predicted_trajectory_sync->read_message(msg);
    // _control_command_sync->read_message();

    read_success |= _continental_objects_sync->read_message(msg);

    read_success |= _tf_sync->read_message(msg);
    read_success |= _tf_static_sync->read_message(msg);

    if (read_success) {
      return;
    }
  }
  _bag_is_empty = true;
}

void SvoRosbagSyncNode::timer_callback()
{
  if (_zed_timestamp_offset == 0ul) {
    return;
  }

  while (!_bag_is_empty) {
    uint64_t now = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    bool publish_success = false;
    publish_success |= _fpa_llh_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _fpa_llh_trans_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _fpa_odometry_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _fpa_odomsh_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _odometry_ecef_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _odometry_llh_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _poiimu_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _rawimu_sync->timer_callback(_zed_timestamp_offset, now, this);

    publish_success |= _velocity_status_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _predicted_trajectory_sync->timer_callback(_zed_timestamp_offset, now, this);

    publish_success |= _continental_objects_sync->timer_callback(_zed_timestamp_offset, now, this);

    publish_success |= _tf_sync->timer_callback(_zed_timestamp_offset, now, this);
    publish_success |= _tf_static_sync->timer_callback(_zed_timestamp_offset, now, this);

    if (publish_success) {
      read_message();
    } else {
      return;
    }
  }
}

void SvoRosbagSyncNode::diagnostics_callback(const diagnostic_msgs::msg::DiagnosticArray & msg)
{
  uint64_t header_timestamp = msg.header.stamp.sec * 1000000000ul + msg.header.stamp.nanosec;
  for (auto status : msg.status) {
    if (status.hardware_id != "Stereolabs camera: zed") {
      continue;
    }

    for (auto keyValue : status.values) {
      if (keyValue.key != "Playing SVO") {
        continue;
      }

      int slash_index = keyValue.value.find('/');
      uint frame = stoi(keyValue.value.substr(7, slash_index - 7));

      if (_zed_timestamps.size() == 0) {
        RCLCPP_ERROR(get_logger(), "No SVO timestamps loaded\n");
        continue;
      }

      if (frame >= _zed_timestamps.size()) {
        continue;
      }

      _zed_timestamp_offset = header_timestamp - _zed_timestamps[frame];

      RCLCPP_INFO(get_logger(), "synchronizing; current offset: %zu\n", _zed_timestamp_offset);
    }
  }
}
