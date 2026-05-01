#include <memory>
#include <rclcpp/rclcpp.hpp>

#include "diagnostics_controller/diagnostics_controller.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<DiagController>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}