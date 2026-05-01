#include <rclcpp/rclcpp.hpp>
#include "session_recorder/session_recorder.hpp"

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<SessionRecorder>();

    try {
        rclcpp::spin(node);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(rclcpp::get_logger("main"), "Exception caught: %s", e.what());
    }

    rclcpp::shutdown();
    return 0;
}
