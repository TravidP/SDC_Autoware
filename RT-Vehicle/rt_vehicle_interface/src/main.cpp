#include <iostream>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <string>

#include "rt_vehicle_interface.hpp"

std::shared_ptr<rclcpp::Node> create_vehicle_node(const std::string & vehicle_type)
{
  static const std::unordered_map<std::string, std::function<std::shared_ptr<rclcpp::Node>()>>
    factory_map = {
      {"citaro", []() { return std::make_shared<GDInterfaceROS2<Citaro>>(); }},
      {"twizy", []() { return std::make_shared<GDInterfaceROS2<Twizy>>(); }}};
  // This map will be extended with the other vehicle types

  auto it = factory_map.find(vehicle_type);
  if (it != factory_map.end()) {
    return it->second();
  } else {
    std::cerr << "Vehicle Interface error: Unsupported vehicle type " << vehicle_type << ".\n";
    return nullptr;
  }
}

int main(int argc, char ** argv)
{
  if (argc < 2) {
    std::cerr << "Vehicle Interface error: Vehicle type needs to be provided.\n";
    return -1;
  }
  std::string vehicle_type = argv[1];
  rclcpp::init(argc, argv);
  auto node = create_vehicle_node(vehicle_type);
  if (node) {
    rclcpp::spin(node);
  }

  rclcpp::shutdown();
  return 0;
}
