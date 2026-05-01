#ifndef ROUTE_GENERATOR_HPP
#define ROUTE_GENERATOR_HPP

#include <tf2/LinearMath/Transform.h>

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "builtin_interfaces/msg/time.hpp"
#include "fixposition_driver_msgs/msg/fpa_llh.hpp"
#include "fixposition_driver_msgs/msg/fpa_odometry.hpp"
#include "nav_msgs/msg/odometry.hpp"

class RouteCreator
{
public:
  void generate_route_from_navsatfix(
    const std::vector<fixposition_driver_msgs::msg::FpaLlh> & fixposition_navsatfix_message,
    const std::string & output_folder);
  void generate_route_ecef_to_base_link(
    const std::vector<nav_msgs::msg::Odometry> & ecef_to_baselink_message,
    const std::string & output_folder) const;

private:
  double convert_time_to_seconds(const builtin_interfaces::msg::Time & time_msg) const noexcept;
  void write_odom_message_to_file(
    const nav_msgs::msg::Odometry & msg, std::ofstream & filestream) const noexcept;
  void write_stamped_transform_to_file(
    const tf2::Transform & transform, double timestamp, std::ofstream & filestream) const noexcept;
};

#endif  // ROUTE_GENERATOR_HPP
