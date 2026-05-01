#include <rclcpp/rclcpp.hpp>
#include "gd_kinematic_state_publisher_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<GDKinematicStatePublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
