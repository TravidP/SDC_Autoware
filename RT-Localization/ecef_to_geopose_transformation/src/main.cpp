#include <memory>
#include <rclcpp/rclcpp.hpp>
#include "ecef_to_geopose_transformation/ecef2geopose_node.hpp"

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Ecef2GeoposeTransform>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
