#include <memory>
#include <rclcpp/rclcpp.hpp>

#include "route_generator/route_generator_node.hpp"

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RouteGenerator>();
    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}
