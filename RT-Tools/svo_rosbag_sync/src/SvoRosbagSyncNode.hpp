#pragma once
#include <vector>

#include "TfTopicSync.hpp"
#include "TopicSync.hpp"
#include "TopicSyncAndConvert.hpp"
#include "autoware_control_msgs/msg/control.hpp"
#include "autoware_planning_msgs/msg/trajectory.hpp"
#include "autoware_vehicle_msgs/msg/steering_report.hpp"
#include "autoware_vehicle_msgs/msg/velocity_report.hpp"
#include "continental_msgs/msg/continental_ars548_object_list.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "fixposition_driver_ros2/msg/llh.hpp"
#include "fixposition_driver_ros2/msg/odometry.hpp"
#include "fixposition_driver_ros2/msg/odomsh.hpp"
#include "geometry_msgs/msg/vector3_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/nav_sat_fix.hpp"
#include "tf2_msgs/msg/tf_message.hpp"

class SvoRosbagSyncNode : public rclcpp::Node
{
public:
  SvoRosbagSyncNode();

private:
  void load_zed_timestamps(const std::string & timestamps_filename);
  void read_message();
  void timer_callback();
  void diagnostics_callback(const diagnostic_msgs::msg::DiagnosticArray & msg);

  rclcpp::TimerBase::SharedPtr _timer;
  rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr _diagnostics_subscription;

  std::shared_ptr<TopicSync<fixposition_driver_ros2::msg::LLH>> _fpa_llh_sync;
  std::shared_ptr<TopicSync<fixposition_driver_ros2::msg::LLH>> _fpa_llh_trans_sync;
  std::shared_ptr<TopicSync<fixposition_driver_ros2::msg::ODOMETRY>> _fpa_odometry_sync;
  std::shared_ptr<TopicSync<fixposition_driver_ros2::msg::ODOMSH>> _fpa_odomsh_sync;
  std::shared_ptr<TopicSync<nav_msgs::msg::Odometry>> _odometry_ecef_sync;
  std::shared_ptr<TopicSync<sensor_msgs::msg::NavSatFix>> _odometry_llh_sync;
  std::shared_ptr<TopicSync<sensor_msgs::msg::Imu>> _poiimu_sync;
  std::shared_ptr<TopicSync<sensor_msgs::msg::Imu>> _rawimu_sync;

  std::shared_ptr<TopicSync<autoware_vehicle_msgs::msg::VelocityReport>> _velocity_status_sync;
  std::shared_ptr<TopicSync<autoware_vehicle_msgs::msg::SteeringReport>> _steering_status_sync;
  std::shared_ptr<TopicSync<autoware_planning_msgs::msg::Trajectory>> _predicted_trajectory_sync;
  // std::shared_ptr<TopicSyncAndConvert<autoware_auto_control_msgs::msg::AckermannControlCommand,
  // autoware_control_msgs::msg::Control>> _control_command_sync;

  std::shared_ptr<TopicSync<continental_msgs::msg::ContinentalArs548ObjectList>>
    _continental_objects_sync;

  std::shared_ptr<TfTopicSync> _tf_sync;
  std::shared_ptr<TfTopicSync> _tf_static_sync;

  rosbag2_cpp::Reader _reader;
  bool _bag_is_empty = false;

  std::vector<uint64_t> _zed_timestamps;
  uint64_t _zed_timestamp_offset = 0ul;
};