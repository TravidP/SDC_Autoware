#ifndef DIAGNOSTICS_CONTROLLER_HPP
#define DIAGNOSTICS_CONTROLLER_HPP

#include <autoware_vehicle_msgs/msg/engage.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>

class DiagController : public rclcpp::Node
{
public:
  DiagController();

private:
  void diag_callback(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);
  void handle_diagnostic(const diagnostic_msgs::msg::DiagnosticStatus & status);
  std::vector<std::string> monitored_names_;
  rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr diag_sub_;
  rclcpp::Publisher<autoware_vehicle_msgs::msg::Engage>::SharedPtr engage_pub_;
};

#endif