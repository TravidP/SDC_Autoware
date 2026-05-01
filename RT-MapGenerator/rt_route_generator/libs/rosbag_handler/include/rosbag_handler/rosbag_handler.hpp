#ifndef ROSBAG_HANDLER_HPP
#define ROSBAG_HANDLER_HPP

#include <string>
#include <utility>
#include <vector>

#include "fixposition_driver_msgs/msg/fpa_llh.hpp"
#include "fixposition_driver_msgs/msg/fpa_odometry.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rosbag2_cpp/reader.hpp"
#include "autoware_map_msgs/msg/map_projector_info.hpp"

class RosbagHandler
{
public:
  struct Result
  {
    std::vector<std::pair<double, double>> navsatfix_coordinates;
    autoware_map_msgs::msg::MapProjectorInfo map_projector_message;
    std::vector<fixposition_driver_msgs::msg::FpaLlh> fixposition_navsatfix_message;
    std::vector<nav_msgs::msg::Odometry> ecef_to_baselink_message;
    std::vector<nav_msgs::msg::Odometry> kinematic_state_message;
    std::vector<fixposition_driver_msgs::msg::FpaOdometry> fixposition_odometry_message;
  };
  Result open_rosbag(const std::string & rosbag_file);

private:
  rosbag2_cpp::Reader rosbag_reader;
  rclcpp::Serialization<fixposition_driver_msgs::msg::FpaLlh> serializer;
  rclcpp::Serialization<nav_msgs::msg::Odometry> serializer_loc;
  rclcpp::Serialization<fixposition_driver_msgs::msg::FpaOdometry> serializer_fix_position_odom;
  rclcpp::Serialization<nav_msgs::msg::Odometry> serializer_ecef_to_baselink_odom;
  rclcpp::Serialization<autoware_map_msgs::msg::MapProjectorInfo> serializer_map;
};

#endif  // ROSBAG_HANDLER_HPP
