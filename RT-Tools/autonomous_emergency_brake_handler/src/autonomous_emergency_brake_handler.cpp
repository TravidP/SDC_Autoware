#include <algorithm>
#include <autoware_adapi_v1_msgs/msg/mrm_state.hpp>
#include <autoware_control_msgs/msg/control.hpp>
#include <autoware_vehicle_msgs/msg/engage.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <iostream>
#include <rclcpp/rclcpp.hpp>

class AEBHandler : public rclcpp::Node
{
public:
  AEBHandler() : Node("gd_autonomous_emergency_brake_node")
  {
    rclcpp::QoS qos_profile(1);
    qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);
    qos_profile.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);
    qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);

    diagnostic_subscriber_ = this->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
      "/control/control_evaluator/metrics", qos_profile,
      std::bind(&AEBHandler::diagnostic_callback, this, std::placeholders::_1));
    control_subscriber_ = this->create_subscription<autoware_control_msgs::msg::Control>(
      "/control/command/control_cmd", qos_profile,
      std::bind(&AEBHandler::control_callback, this, std::placeholders::_1));
    emergency_control_publisher_ = this->create_publisher<autoware_control_msgs::msg::Control>(
      "/system/emergency/control_cmd", qos_profile);
    engage_publisher_ =
      this->create_publisher<autoware_vehicle_msgs::msg::Engage>("/autoware/engage", qos_profile);
    mrm_state_publisher_ = this->create_publisher<autoware_adapi_v1_msgs::msg::MrmState>(
      "/system/fail_safe/mrm_state", qos_profile);
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100), std::bind(&AEBHandler::publish_timer_callback, this));

    control_msg.longitudinal.velocity = 0.0;
    control_msg.longitudinal.acceleration = declare_parameter<float>("aeb_acceleration");
    control_msg.longitudinal.jerk = declare_parameter<float>("aeb_jerk");  // Check if needed
  }

private:
  void control_callback(const autoware_control_msgs::msg::Control::SharedPtr msg)
  {
    control_msg.lateral = msg->lateral;
  }
  void diagnostic_callback(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg)
  {
    auto it = std::find_if(msg->status.begin(), msg->status.end(), [](const auto & status) {
      return status.name == "autonomous_emergency_braking: aeb_emergency_stop";
    });

    if (it != msg->status.end()) {
      for (const auto & message : it->values) {
        if (message.value.find("deceleration") != std::string::npos) {
          RCLCPP_INFO_STREAM(this->get_logger(), "Received AEB message: " << message.value.c_str());
          aeb_pub_ = true;

          autoware_vehicle_msgs::msg::Engage engage_msg;
          engage_msg.stamp = this->get_clock()->now();
          engage_msg.engage = false;
          engage_publisher_->publish(engage_msg);
        } else {
          aeb_pub_ = false;
        }
      }
    }
  }

  void publish_timer_callback()
  {
    autoware_adapi_v1_msgs::msg::MrmState mrm_state_msg;
    mrm_state_msg.stamp = this->get_clock()->now();

    if (!aeb_pub_) {
      mrm_state_msg.state = 1;     // NORMAL
      mrm_state_msg.behavior = 1;  // NONE
    } else {
      mrm_state_msg.state = 2;     // MRM_OPERATING
      mrm_state_msg.behavior = 2;  // EMERGENCY_STOP

      control_msg.stamp = this->get_clock()->now();
      control_msg.longitudinal.stamp = this->get_clock()->now();
      emergency_control_publisher_->publish(control_msg);
    }
    mrm_state_publisher_->publish(mrm_state_msg);
  }

  rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr diagnostic_subscriber_;
  rclcpp::Subscription<autoware_control_msgs::msg::Control>::SharedPtr control_subscriber_;
  rclcpp::Publisher<autoware_control_msgs::msg::Control>::SharedPtr emergency_control_publisher_;
  rclcpp::Publisher<autoware_vehicle_msgs::msg::Engage>::SharedPtr engage_publisher_;
  rclcpp::Publisher<autoware_adapi_v1_msgs::msg::MrmState>::SharedPtr mrm_state_publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  autoware_control_msgs::msg::Control control_msg;
  bool aeb_pub_ = false;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<AEBHandler>());
  rclcpp::shutdown();
  return 0;
}
