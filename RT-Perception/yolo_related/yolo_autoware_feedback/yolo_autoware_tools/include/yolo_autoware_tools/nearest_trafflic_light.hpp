#ifndef NEAREST_TRAFFIC_LIGHT_HPP
#define NEAREST_TRAFFIC_LIGHT_HPP

#include <cmath>
#include <limits>
#include <nav_msgs/msg/odometry.hpp>
#include <pugixml.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "autoware_lanelet2_extension/projection/transverse_mercator_projector.hpp"
#include "yaml-cpp/yaml.h"

class NearestTrafficLight : public rclcpp::Node
{
public:
  NearestTrafficLight();

private:
  void localization_callback(const nav_msgs::msg::Odometry::SharedPtr msg);
  double calculate_distance(double lat1, double lon1, double lat2, double lon2);
  double degrees_to_radians(double degrees);

  struct RelationInfo
  {
    std::string id;
    std::vector<std::pair<double, double>> ref_line;
    std::vector<std::pair<double, double>> light_bulbs;
    std::vector<std::pair<double, double>> refers;
  };

  std::vector<RelationInfo> extract_traffic_light_info(const std::string & file_path);
  void publish_traffic_light_info();

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr subscription_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::QoS qos_profile_{1};

  double vehicle_lat_;
  double vehicle_lon_;
  double vehicle_ele_;

  std::string map_path_;
  std::vector<RelationInfo> traffic_light_info_;
  lanelet::projection::TransverseMercatorProjector _projector;
};

#endif  // NEAREST_TRAFFIC_LIGHT_HPP
