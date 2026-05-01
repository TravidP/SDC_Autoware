// Copyright 2021 Tier IV, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GD_VEHICLE_INTERFACE_PANEL_HPP_
#define GD_VEHICLE_INTERFACE_PANEL_HPP_

#include <rviz_common/display_context.hpp>
#include <rviz_common/panel.hpp>
#include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>

#include <tier4_vehicle_msgs/msg/battery_status.hpp>
#include <tier4_vehicle_msgs/msg/vehicle_emergency_stamped.hpp>
#include <tier4_external_api_msgs/msg/heartbeat.hpp>
#include <autoware_vehicle_msgs/msg/control_mode_report.hpp>
#include <autoware_vehicle_msgs/msg/gear_report.hpp>
#include <autoware_vehicle_msgs/msg/gear_command.hpp>
#include <autoware_vehicle_msgs/msg/velocity_report.hpp>
#include <autoware_vehicle_msgs/msg/steering_report.hpp>
#include <autoware_vehicle_msgs/msg/hazard_lights_report.hpp>
#include <autoware_vehicle_msgs/msg/hazard_lights_command.hpp>
#include <autoware_vehicle_msgs/msg/turn_indicators_report.hpp>
#include <autoware_vehicle_msgs/msg/turn_indicators_command.hpp>

#include <QTimer>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <rclcpp/rclcpp.hpp>


class QLineEdit;
class QLabel;
namespace rviz_plugins
{
class GDVehicleInterfacePanel : public rviz_common::Panel
{
  Q_OBJECT

public:
  explicit GDVehicleInterfacePanel(QWidget * parent = nullptr);
  void onInitialize() override;
  void checkMessageTimeouts();
  void updateLabels();

public Q_SLOTS:  
  void onClickEmergencyButton();
  void onClickSetGearButton();
  void onClickSetHazardLightButton();
  void onClickSetLeftTurnButton();
  void onClickSetRightTurnButton();

protected:
  QGroupBox * makeVehicleInterfaceObserveGroup();
  QGroupBox * makeGDControlModeGroup();

  rclcpp::Node::SharedPtr raw_node_;
  rclcpp::Clock::SharedPtr clock_;
  QString defaultText_ = "NO DATA";

  QLineEdit * battery_status_label_;
  QString batteryLevelText_;
  rclcpp::Time last_battery_status_time_;
  rclcpp::Subscription<tier4_vehicle_msgs::msg::BatteryStatus>::SharedPtr sub_battery_status_;
  void onBatteryStatus(const tier4_vehicle_msgs::msg::BatteryStatus::ConstSharedPtr msg);

  QLineEdit * heartbeat_label_;
  QString heartbeatStatusText_;
  rclcpp::Subscription<tier4_external_api_msgs::msg::Heartbeat>::SharedPtr sub_heartbeat_status_;
  rclcpp::Time last_heartbeat_status_time_;
  void onHeartbeatStatus(const tier4_external_api_msgs::msg::Heartbeat::ConstSharedPtr msg);

  QLineEdit * control_mode_label_;
  QString controlModeStatusText_ = "NO DATA";
  rclcpp::Subscription<autoware_vehicle_msgs::msg::ControlModeReport>::SharedPtr sub_control_mode_status_;
  void onControlModeStatus(const autoware_vehicle_msgs::msg::ControlModeReport::ConstSharedPtr msg);
  
  QLineEdit * gear_status_label_;
  QString gearStatusText_ = "NO DATA";
  rclcpp::Subscription<autoware_vehicle_msgs::msg::GearReport>::SharedPtr sub_gear_status_;
  void onGearStatus(const autoware_vehicle_msgs::msg::GearReport::ConstSharedPtr msg);

  QLineEdit * velocity_status_label_;
  QString longVelocityStatusText_ = "NO DATA";
  QString latVelocityStatusText_ = "NO DATA";
  QString headingRateStatusText_ = "NO DATA";
  rclcpp::Subscription<autoware_vehicle_msgs::msg::VelocityReport>::SharedPtr sub_velocity_status_;
  void onVelocityStatus(const autoware_vehicle_msgs::msg::VelocityReport::ConstSharedPtr msg);

  QLineEdit * steering_status_label_;
  QString steeringStatusText_ = "NO DATA";
  rclcpp::Subscription<autoware_vehicle_msgs::msg::SteeringReport>::SharedPtr sub_steering_status_;
  void onSteeringStatus(const autoware_vehicle_msgs::msg::SteeringReport::ConstSharedPtr msg);

  QPushButton* hazard_light_status_label_;
  uint current_hazard_light_{autoware_vehicle_msgs::msg::HazardLightsReport::DISABLE};
  rclcpp::Time last_hazard_status_time_;
  rclcpp::Subscription<autoware_vehicle_msgs::msg::HazardLightsReport>::SharedPtr sub_hazard_light_status_;
  void onHazardLightStatus(const autoware_vehicle_msgs::msg::HazardLightsReport::ConstSharedPtr msg);
  rclcpp::Publisher<autoware_vehicle_msgs::msg::HazardLightsCommand>::SharedPtr pub_hazard_light_command_;

  QPushButton* left_light_indicator_status_label_;
  QPushButton* right_light_indicator_status_label_;
  uint current_turn_light_{autoware_vehicle_msgs::msg::TurnIndicatorsReport::DISABLE};
  rclcpp::Time last_left_light_indicator_status_time_;
  rclcpp::Time last_right_light_indicator_status_time_;
  rclcpp::Subscription<autoware_vehicle_msgs::msg::TurnIndicatorsReport>::SharedPtr sub_turn_light_status_;
  void onTurnLightStatus(const autoware_vehicle_msgs::msg::TurnIndicatorsReport::ConstSharedPtr msg);
  rclcpp::Publisher<autoware_vehicle_msgs::msg::TurnIndicatorsCommand>::SharedPtr pub_turn_light_command_;

 
  QPushButton * gear_button_ptr_;
  QSpinBox * pub_gear_cmd_input_;
  rclcpp::Publisher<autoware_vehicle_msgs::msg::GearCommand>::SharedPtr pub_gear_command_;

  QPushButton * emergency_button_ptr_;
  bool current_vehicle_emergency_{false};
  rclcpp::Subscription<tier4_vehicle_msgs::msg::VehicleEmergencyStamped>::SharedPtr sub_vehicle_emergency_;
  void onVehicleEmergencyStatus(const tier4_vehicle_msgs::msg::VehicleEmergencyStamped::ConstSharedPtr msg);
  rclcpp::Publisher<tier4_vehicle_msgs::msg::VehicleEmergencyStamped>::SharedPtr pub_vehicle_emergency_;

};

}  // namespace rviz_plugins

#endif  // AUTOWARE_DATETIME_PANEL_HPP_
