#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <string>

#include <autoware_vehicle_info_utils/vehicle_info_utils.hpp>

#include "rosbag_recorder/rosbag_recorder.hpp"
#include "rosbag_handler/rosbag_handler.hpp"
#include "route_creator/route_creator.hpp"
#include "map_generator/map_generator.hpp"

#include "rt_route_generator_msgs/srv/create_map.hpp"
#include "rt_route_generator_msgs/srv/record.hpp"

class RouteGenerator : public rclcpp::Node

{
public:
  RouteGenerator();

private:
  void create_map_callback(
    const std::shared_ptr<rt_route_generator_msgs::srv::CreateMap::Request> request,
    std::shared_ptr<rt_route_generator_msgs::srv::CreateMap::Response> response);

  void record_callback(
    const std::shared_ptr<rt_route_generator_msgs::srv::Record::Request> request,
    std::shared_ptr<rt_route_generator_msgs::srv::Record::Response> response);

  const std::string get_current_datetime() const;

  rclcpp::Service<rt_route_generator_msgs::srv::CreateMap>::SharedPtr create_map_server_;
  rclcpp::Service<rt_route_generator_msgs::srv::Record>::SharedPtr record_server_;

  std::unique_ptr<RosbagRecorder> rosbag_recorder_;
  std::unique_ptr<RosbagHandler> rosbag_handler_;
  std::unique_ptr<RouteCreator> route_creator_;
  std::unique_ptr<MapGenerator> map_generator_;

  std::string last_bag_name_;
  autoware::vehicle_info_utils::VehicleInfo vehicle_info_;
  //Parameters
  std::string map_directory_;
  std::string bag_directory_;
  std::vector<std::string> topic_names_;
};
