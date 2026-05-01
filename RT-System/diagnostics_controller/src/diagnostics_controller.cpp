#include "diagnostics_controller/diagnostics_controller.hpp"

#include <autoware_vehicle_msgs/msg/engage.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>

using std::placeholders::_1;

DiagController::DiagController() : rclcpp::Node("diagnostics_controller")
{
  rclcpp::QoS qos_profile(1);
  qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);
  qos_profile.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);
  qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);
  engage_pub_ =
    this->create_publisher<autoware_vehicle_msgs::msg::Engage>("/autoware/engage", qos_profile);
  diag_sub_ = create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
    "/diagnostics", 10, std::bind(&DiagController::diag_callback, this, _1));
  monitored_names_ = this->declare_parameter<std::vector<std::string>>("monitored_names", {});
}

void DiagController::diag_callback(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg)
{
  for (const auto & status : msg->status) {
    // Find which substring matches (if any)
    auto match_it = std::find_if(
      monitored_names_.begin(), monitored_names_.end(), [&status](const std::string & substring) {
        return status.name.find(substring) != std::string::npos;
      });

    if (match_it != monitored_names_.end()) {
      handle_diagnostic(status);
    }
  }
}

void DiagController::handle_diagnostic(const diagnostic_msgs::msg::DiagnosticStatus & status)
{
  RCLCPP_INFO_STREAM(this->get_logger(), "Name: " << status.name);
  if (status.level == 0)  //OK
  {
    RCLCPP_INFO_STREAM(this->get_logger(), "Level: OK");
  } else if (status.level == 1)  //WARN
  {
    RCLCPP_INFO_STREAM(this->get_logger(), "Level: WARN");
  } else if (status.level == 2)  //ERROR
  {
    RCLCPP_INFO_STREAM(this->get_logger(), "Level: ERROR. Disengaging autoware control");
    autoware_vehicle_msgs::msg::Engage disengage_msg;
    disengage_msg.stamp = this->get_clock()->now();
    disengage_msg.engage = false;
    engage_pub_->publish(disengage_msg);
  } else {
    RCLCPP_INFO_STREAM(this->get_logger(), "SHOULD NOT reach this level");
    autoware_vehicle_msgs::msg::Engage disengage_msg;
    disengage_msg.stamp = this->get_clock()->now();
    disengage_msg.engage = false;
    engage_pub_->publish(disengage_msg);
  }
}
