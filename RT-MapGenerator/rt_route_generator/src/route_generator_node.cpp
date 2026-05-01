#include "route_generator/route_generator_node.hpp"

#include <autoware_vehicle_info_utils/vehicle_info_utils.hpp>
#include <filesystem>

#include "map_generator/map_generator.hpp"
#include "rosbag_handler/rosbag_handler.hpp"
#include "rosbag_recorder/rosbag_recorder.hpp"
#include "route_creator/route_creator.hpp"
#include "rt_route_generator_msgs/srv/create_map.hpp"
#include "rt_route_generator_msgs/srv/record.hpp"

//Time functionality
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

RouteGenerator::RouteGenerator() : rclcpp::Node("route_generator")

{
  create_map_server_ = create_service<rt_route_generator_msgs::srv::CreateMap>(
    "/create_map",
    std::bind(
      &RouteGenerator::create_map_callback, this, std::placeholders::_1, std::placeholders::_2));
  record_server_ = create_service<rt_route_generator_msgs::srv::Record>(
    "/record",
    std::bind(
      &RouteGenerator::record_callback, this, std::placeholders::_1, std::placeholders::_2));

  bag_directory_ = declare_parameter<std::string>("bag_directory");
  map_directory_ = declare_parameter<std::string>("map_directory");
  if (!std::filesystem::exists(map_directory_)) {
    std::filesystem::create_directories(map_directory_);
  }
  topic_names_ = declare_parameter<std::vector<std::string>>("topic_names");

  rosbag_recorder_ = std::make_unique<RosbagRecorder>(bag_directory_);
  rosbag_handler_ = std::make_unique<RosbagHandler>();
  route_creator_ = std::make_unique<RouteCreator>();
  vehicle_info_ = autoware::vehicle_info_utils::VehicleInfoUtils(*this).getVehicleInfo();

  MapGeneratorOptions opts;
  opts.speed_limit = declare_parameter<double>("speed_limit");
  opts.lane_width = declare_parameter<double>("lane_width");
  opts.max_nodes_in_way = declare_parameter<int>("max_nodes_in_way");
  opts.averaging_window_size = declare_parameter<int>("averaging_window_size");
  opts.loop = declare_parameter<bool>("loop");
  opts.base_to_front = vehicle_info_.wheel_base_m + vehicle_info_.front_overhang_m;
  map_generator_ = std::make_unique<MapGenerator>(opts);

  RCLCPP_INFO(this->get_logger(), "Route generator server online.");
}

void RouteGenerator::create_map_callback(
  const std::shared_ptr<rt_route_generator_msgs::srv::CreateMap::Request> request,
  std::shared_ptr<rt_route_generator_msgs::srv::CreateMap::Response> response)
{
  if (request->create_map) {
    std::string bag_name = "/" + last_bag_name_ + "_0.db3";
    std::string rosbag_name = bag_directory_ + last_bag_name_ + bag_name;
    RCLCPP_INFO_STREAM(this->get_logger(), "Handling rosbag " << rosbag_name);
    auto result = rosbag_handler_->open_rosbag(rosbag_name);
    RCLCPP_INFO(this->get_logger(), "Creating map...");
    //Checks whether the handling produced correct results, these need to be refactored in the rosbag_handler
    if (result.navsatfix_coordinates.empty()) {
      response->success = false;
      response->message = "NavSatFix coordinates are empty.";
    } else if (result.fixposition_navsatfix_message.empty()) {
      response->success = false;
      response->message = "Fixposition NavSatFix coordinates are empty.";
    } else if (result.ecef_to_baselink_message.empty()) {
      response->success = false;
      response->message = "ECEF to base_link messages are empty.";
    } else {
      std::string map_name = map_directory_ + "/" + last_bag_name_;
      if (!std::filesystem::exists(map_name)) {
        std::filesystem::create_directories(map_name);
      }
      route_creator_->generate_route_ecef_to_base_link(result.ecef_to_baselink_message, map_name);
      route_creator_->generate_route_from_navsatfix(result.fixposition_navsatfix_message, map_name);
      map_generator_->generate_lanelet_map(result.navsatfix_coordinates, map_name);
      map_generator_->generate_map_projector(result.navsatfix_coordinates, map_name);
      response->success = true;
      response->message = "Map created.";
    }
  } else {
    RCLCPP_INFO(this->get_logger(), "Not creating map...");
    response->success = false;
    response->message = "Map NOT created.";
  }
}

void RouteGenerator::record_callback(
  const std::shared_ptr<rt_route_generator_msgs::srv::Record::Request> request,
  std::shared_ptr<rt_route_generator_msgs::srv::Record::Response> response)
{
  if (request->record) {
    if (request->rec_name.empty()) {
      // name of the bag is date_time
      auto bag_name = get_current_datetime();
      auto res = rosbag_recorder_->startRecording(topic_names_, bag_name);
      response->success = res.success_;
      response->message = res.msg_;
      RCLCPP_INFO(this->get_logger(), res.msg_.c_str());
      last_bag_name_ = bag_name;
    } else {
      std::string output_name = bag_directory_ + request->rec_name;
      auto res = rosbag_recorder_->startRecording(topic_names_, output_name);
      response->success = res.success_;
      response->message = res.msg_;
      RCLCPP_INFO(this->get_logger(), res.msg_.c_str());
      last_bag_name_ = request->rec_name;
    }
  } else {
    auto res = rosbag_recorder_->stopRecording();
    response->success = res.success_;
    response->message = res.msg_;
    RCLCPP_INFO(this->get_logger(), res.msg_.c_str());
  }
}

const std::string RouteGenerator::get_current_datetime() const
{
  std::time_t t = std::time(nullptr);
  std::tm * now = std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(now, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}
