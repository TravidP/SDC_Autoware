#include <autoware_control_msgs/msg/control.hpp>
#include <autoware_vehicle_msgs/msg/gear_command.hpp>
#include <autoware_vehicle_msgs/msg/turn_indicators_command.hpp>
#include <chrono>
#include <memory>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <std_msgs/msg/float32.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <std_msgs/msg/int64.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <thread>
#include <tier4_external_api_msgs/msg/heartbeat.hpp>

#include "ros_vehicle_adapter.hpp"

using std::placeholders::_1;

using namespace std::chrono_literals;

using ControlMessage = autoware_control_msgs::msg::Control;
using GearCommand = autoware_vehicle_msgs::msg::GearCommand;
using TurnIndicatorsCommand = autoware_vehicle_msgs::msg::TurnIndicatorsCommand;

// TODO make the sub-callback publishing to the LLC more general by adding
// static specific subs depending on the vehicle.
//  Add MultiThreaded Executor in main and separate callbacks in groups.
//  Make heartbeat period a param.
//  Make deadline for steering and speed a param.
template <typename T>
class GDInterfaceROS2 : public rclcpp::Node
{
public:
  GDInterfaceROS2() : Node("vehicle_interface")
  {
    create_sub_pub();

    vehicle_adapter_ = std::make_unique<RosVehicleAdapter<T>>();
    heartbeat_timer_ = create_wall_timer(
      std::chrono::milliseconds(300), std::bind(&GDInterfaceROS2::publish_heartbeat, this));
  }

private:
  void ackermann_cc_cb(const ControlMessage::ConstSharedPtr msg)
  {
    std_msgs::msg::Float32MultiArray command_msg;
    command_msg.data.resize(2);
    command_msg.data[0] = vehicle_adapter_->handleSpeed(msg->longitudinal.velocity);
    command_msg.data[1] = msg->longitudinal.acceleration;
    target_speed_acc_pub_->publish(command_msg);
    std_msgs::msg::Int64 steering_command_msg;
    steering_command_msg.data = vehicle_adapter_->handleSteering(msg->lateral.steering_tire_angle);
    target_steering_angle_pub_->publish(steering_command_msg);
  }

  void gear_command_cb(const GearCommand::ConstSharedPtr msg)
  {
    std_msgs::msg::UInt8 command_msg;
    command_msg.data = vehicle_adapter_->handleGear(msg->command);
    gear_command_pub_->publish(command_msg);
  }

  void indicators_command_cb(const TurnIndicatorsCommand::ConstSharedPtr msg)
  {
    std_msgs::msg::UInt8 command_msg;
    command_msg.data = vehicle_adapter_->handleIndicators(msg->command);
    indicator_command_pub_->publish(command_msg);
  }

  void create_sub_pub()
  {
    rclcpp::QoS qos_profile(1);         // Keep last message
    qos_profile.best_effort();          // Avoid retransmission
    qos_profile.durability_volatile();  // No message retention for late joiners
    auto deadline_ns = rclcpp::Duration(std::chrono::nanoseconds(50000000));  // 50ms
    qos_profile.deadline(deadline_ns);

    ack_cc_sub_ = create_subscription<ControlMessage>(
      "/control/command/control_cmd", 1, std::bind(&GDInterfaceROS2<T>::ackermann_cc_cb, this, _1));
    gear_command_sub_ = create_subscription<GearCommand>(
      "/control/command/gear_cmd", 1, std::bind(&GDInterfaceROS2<T>::gear_command_cb, this, _1));
    indicators_command_sub_ = create_subscription<TurnIndicatorsCommand>(
      "/control/command/turn_indicators_cmd", 1,
      std::bind(&GDInterfaceROS2<T>::indicators_command_cb, this, _1));

    rclcpp::PublisherOptions target_speed_publisher_options;
    target_speed_publisher_options.event_callbacks.deadline_callback =
      [this](rclcpp::QOSDeadlineOfferedInfo & event) {
        RCLCPP_WARN_STREAM(
          this->get_logger(), "Deadline missed: total missed so far = "
                                << event.total_count << " by speed target publisher");
      };

    target_speed_acc_pub_ = create_publisher<std_msgs::msg::Float32MultiArray>(
      "/vehicle_controller/target_speed_acc", qos_profile, target_speed_publisher_options);

    rclcpp::PublisherOptions target_steering_publisher_options;
    target_steering_publisher_options.event_callbacks.deadline_callback =
      [this](rclcpp::QOSDeadlineOfferedInfo & event) {
        RCLCPP_WARN_STREAM(
          this->get_logger(), "Deadline missed: total missed so far = "
                                << event.total_count << " by steering target publisher");
      };
    target_steering_angle_pub_ = create_publisher<std_msgs::msg::Int64>(
      "/vehicle_controller/target_steering_angle", qos_profile, target_steering_publisher_options);
    indicator_command_pub_ =
      create_publisher<std_msgs::msg::UInt8>("/vehicle_controller/indicator_command", qos_profile);
    gear_command_pub_ =
      create_publisher<std_msgs::msg::UInt8>("/vehicle_controller/gear_command", qos_profile);

    heartbeat_pub_ = create_publisher<tier4_external_api_msgs::msg::Heartbeat>(
      "vehicle/interface_heartbeat", rclcpp::QoS{1});
  }

  void publish_heartbeat()
  {
    tier4_external_api_msgs::msg::Heartbeat heartbeat_msg;
    heartbeat_msg.stamp = this->now();
    heartbeat_pub_->publish(heartbeat_msg);
  }

  // Subscribers
  rclcpp::Subscription<ControlMessage>::SharedPtr ack_cc_sub_;
  rclcpp::Subscription<GearCommand>::SharedPtr gear_command_sub_;
  rclcpp::Subscription<TurnIndicatorsCommand>::SharedPtr indicators_command_sub_;

  // Publishers to LLC
  rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr target_speed_acc_pub_;
  rclcpp::Publisher<std_msgs::msg::Int64>::SharedPtr target_steering_angle_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr indicator_command_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr gear_command_pub_;

  rclcpp::Publisher<tier4_external_api_msgs::msg::Heartbeat>::SharedPtr heartbeat_pub_;

  // Heartbeat timer
  rclcpp::TimerBase::SharedPtr heartbeat_timer_;

  std::unique_ptr<RosVehicleAdapter<T>> vehicle_adapter_;
};
