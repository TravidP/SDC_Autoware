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

#include "greendino_vehicle_interface_panel.hpp"
#include <QVBoxLayout>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QTimer>
#include <QWidget>
#include <rclcpp/rclcpp.hpp>

#include <ctime>

namespace rviz_plugins
{
GDVehicleInterfacePanel::GDVehicleInterfacePanel(QWidget * parent) : rviz_common::Panel(parent)
{
  // Layout
  auto * v_layout = new QVBoxLayout;
  v_layout->addWidget(makeVehicleInterfaceObserveGroup());
  v_layout->addWidget(makeGDControlModeGroup());
  setLayout(v_layout);

  QTimer *updateTimer = new QTimer(this);
  connect(updateTimer, &QTimer::timeout, this, &GDVehicleInterfacePanel::updateLabels);
  updateTimer->start(100);

  clock_ = std::make_shared<rclcpp::Clock>(RCL_SYSTEM_TIME); // Or RCL_ROS_TIME, depending on your use case

  QTimer *messageCheckTimer = new QTimer(this);
  connect(messageCheckTimer, &QTimer::timeout, this, &GDVehicleInterfacePanel::checkMessageTimeouts);
  messageCheckTimer->start(1000); 
}

QGroupBox * GDVehicleInterfacePanel::makeVehicleInterfaceObserveGroup()
{
  auto * group = new QGroupBox("GD - Vehicle Interface Status");
  auto * grid = new QGridLayout;

  battery_status_label_ = new QLineEdit;
  battery_status_label_->setReadOnly(true);

  heartbeat_label_ = new QLineEdit;
  heartbeat_label_->setReadOnly(true);

  control_mode_label_ = new QLineEdit;
  control_mode_label_->setReadOnly(true);

  gear_status_label_ = new QLineEdit;
  gear_status_label_->setReadOnly(true);

  velocity_status_label_ = new QLineEdit;
  velocity_status_label_->setReadOnly(true);

  steering_status_label_ = new QLineEdit;
  steering_status_label_->setReadOnly(true);

  hazard_light_status_label_ = new QPushButton(this);
  hazard_light_status_label_->setFixedSize(100, 50);
  // hazard_light_status_label_->setAlignment(Qt::AlignCenter);
  hazard_light_status_label_->setStyleSheet("background-color: white;");
  connect(hazard_light_status_label_, SIGNAL(clicked()), this, SLOT(onClickSetHazardLightButton()));

  QWidget *containerWidget = new QWidget(this);
  QHBoxLayout *containerLayout = new QHBoxLayout(containerWidget);
  containerLayout->setContentsMargins(0, 0, 0, 0);
  left_light_indicator_status_label_ = new QPushButton(this);
  left_light_indicator_status_label_->setFixedSize(100, 50);
  // left_light_indicator_status_label_->setAlignment(Qt::AlignCenter);
  left_light_indicator_status_label_->setStyleSheet("background-color: white;");
  connect(left_light_indicator_status_label_, SIGNAL(clicked()), this, SLOT(onClickSetLeftTurnButton()));

  right_light_indicator_status_label_ = new QPushButton(this);
  right_light_indicator_status_label_->setFixedSize(100, 50);
  // right_light_indicator_status_label_->setAlignment(Qt::AlignCenter);
  right_light_indicator_status_label_->setStyleSheet("background-color: white;");
  connect(right_light_indicator_status_label_, SIGNAL(clicked()), this, SLOT(onClickSetRightTurnButton()));

  containerLayout->addWidget(left_light_indicator_status_label_);
  containerLayout->addWidget(right_light_indicator_status_label_);

  grid->addWidget(new QLabel("Battery Status:"), 0, 0);
  grid->addWidget(battery_status_label_, 0, 1);
  grid->addWidget(new QLabel("Heartbeat Status:"), 0, 2);
  grid->addWidget(heartbeat_label_, 0, 3);
  grid->addWidget(new QLabel("Control Mode Status:"), 1, 0);
  grid->addWidget(control_mode_label_, 1, 1);
  grid->addWidget(new QLabel("Gear Status:"), 1, 2);
  grid->addWidget(gear_status_label_, 1, 3);
  grid->addWidget(new QLabel("Velocity Status:"), 2, 0);
  grid->addWidget(velocity_status_label_, 2, 1);
  grid->addWidget(new QLabel("Steering Status:"), 2, 2);
  grid->addWidget(steering_status_label_, 2, 3);
  grid->addWidget(new QLabel("Hazard Light Status:"), 3, 0);
  grid->addWidget(hazard_light_status_label_, 3, 1); // Add it to your layout at the desired position
  grid->addWidget(new QLabel("Turn Light Status:"), 3, 2);
  grid->addWidget(containerWidget, 3, 3);
  
  group->setLayout(grid);
  return group;
}

QGroupBox * GDVehicleInterfacePanel::makeGDControlModeGroup()
{
  auto * group = new QGroupBox("GD - Vehicle Interface Command");
  auto * grid = new QGridLayout;

  emergency_button_ptr_ = new QPushButton("Send Emergency Command");
  emergency_button_ptr_->setStyleSheet("background-color: white;");
  connect(emergency_button_ptr_, SIGNAL(clicked()), this, SLOT(onClickEmergencyButton()));

  
  gear_button_ptr_ = new QPushButton("Set Gear Command");
  pub_gear_cmd_input_ = new QSpinBox();
  connect(gear_button_ptr_, SIGNAL(clicked()), this, SLOT(onClickSetGearButton()));

  grid->addWidget(emergency_button_ptr_, 0, 0);
  grid->addWidget(pub_gear_cmd_input_, 1, 0);
  grid->addWidget(gear_button_ptr_, 1, 1);
  
  group->setLayout(grid);
  return group;
}

void GDVehicleInterfacePanel::onInitialize()
{
  using std::placeholders::_1;
  raw_node_ = this->getDisplayContext()->getRosNodeAbstraction().lock()->get_raw_node();

  sub_battery_status_ = raw_node_->create_subscription<tier4_vehicle_msgs::msg::BatteryStatus>(
    "/vehicle/status/battery_level", 10, std::bind(&GDVehicleInterfacePanel::onBatteryStatus, this, _1));

  sub_heartbeat_status_ = raw_node_->create_subscription<tier4_external_api_msgs::msg::Heartbeat>(
    "/vehicle/interface_heartbeat", 10, std::bind(&GDVehicleInterfacePanel::onHeartbeatStatus, this, _1));

  sub_control_mode_status_ = raw_node_->create_subscription<autoware_vehicle_msgs::msg::ControlModeReport>(
    "/vehicle/status/control_mode", 10, std::bind(&GDVehicleInterfacePanel::onControlModeStatus, this, _1));
    
  sub_gear_status_ = raw_node_->create_subscription<autoware_vehicle_msgs::msg::GearReport>(
    "/vehicle/status/gear_status", 10, std::bind(&GDVehicleInterfacePanel::onGearStatus, this, _1));

  sub_velocity_status_ = raw_node_->create_subscription<autoware_vehicle_msgs::msg::VelocityReport>(
    "/vehicle/status/velocity_status", 10, std::bind(&GDVehicleInterfacePanel::onVelocityStatus, this, _1));
  
  sub_steering_status_ = raw_node_->create_subscription<autoware_vehicle_msgs::msg::SteeringReport>(
    "/vehicle/status/steering_status", 10, std::bind(&GDVehicleInterfacePanel::onSteeringStatus, this, _1));

  sub_hazard_light_status_ = raw_node_->create_subscription<autoware_vehicle_msgs::msg::HazardLightsReport>(
    "/vehicle/status/hazard_lights_status", 10, std::bind(&GDVehicleInterfacePanel::onHazardLightStatus, this, _1));

  pub_hazard_light_command_ = raw_node_->create_publisher<autoware_vehicle_msgs::msg::HazardLightsCommand>(
    "/external/selected/hazard_lights_cmd", rclcpp::QoS{1}.transient_local());

  sub_turn_light_status_ = raw_node_->create_subscription<autoware_vehicle_msgs::msg::TurnIndicatorsReport>(
    "/vehicle/status/turn_indicators_status", 10, std::bind(&GDVehicleInterfacePanel::onTurnLightStatus, this, _1));
  
  pub_turn_light_command_ = raw_node_->create_publisher<autoware_vehicle_msgs::msg::TurnIndicatorsCommand>(
    "/external/selected/turn_indicators_cmd", rclcpp::QoS{1}.transient_local());

  sub_vehicle_emergency_ = raw_node_->create_subscription<tier4_vehicle_msgs::msg::VehicleEmergencyStamped>(
    "/control/command/emergency_cmd", 10, std::bind(&GDVehicleInterfacePanel::onVehicleEmergencyStatus, this, _1));
  
  pub_gear_command_ = raw_node_->create_publisher<autoware_vehicle_msgs::msg::GearCommand>(
    "/control/command/gear_cmd", rclcpp::QoS{1}.transient_local());

  pub_vehicle_emergency_ = raw_node_->create_publisher<tier4_vehicle_msgs::msg::VehicleEmergencyStamped>(
    "/control/command/emergency_cmd", rclcpp::QoS{1}.transient_local());

}

void GDVehicleInterfacePanel::onBatteryStatus(
  const tier4_vehicle_msgs::msg::BatteryStatus::ConstSharedPtr msg)
{
  auto current_energy_level_ = msg->energy_level;
  batteryLevelText_ = QString::number(current_energy_level_, 'f', 2) + "%";

  last_battery_status_time_ = clock_->now();
}

void GDVehicleInterfacePanel::onHeartbeatStatus(
  const tier4_external_api_msgs::msg::Heartbeat::ConstSharedPtr msg)
{
    if (msg->stamp.sec == 0 && msg->stamp.nanosec == 0) {
        heartbeatStatusText_ = "UNSTABLE";
    }
    else {
        heartbeatStatusText_ = "STABLE";
    }

    last_heartbeat_status_time_ = clock_->now();
}

void GDVehicleInterfacePanel::onControlModeStatus(
  const autoware_vehicle_msgs::msg::ControlModeReport::ConstSharedPtr msg)
{
    switch (msg->mode) {
        case autoware_vehicle_msgs::msg::ControlModeReport::AUTONOMOUS:
            controlModeStatusText_ = "AUTONOMOUS";
            break;
    }
}

void GDVehicleInterfacePanel::onGearStatus(
  const autoware_vehicle_msgs::msg::GearReport::ConstSharedPtr msg)
{
    auto current_gear_status_ = msg->report;
    gearStatusText_ = QString::number(current_gear_status_); 
}

void GDVehicleInterfacePanel::onVelocityStatus(
  const autoware_vehicle_msgs::msg::VelocityReport::ConstSharedPtr msg)
{
    auto current_longitudinal_velocity_ = msg->longitudinal_velocity;
    longVelocityStatusText_ = QString::number(current_longitudinal_velocity_, 'f'); 
    auto current_lateral_velocity_ = msg->lateral_velocity;
    latVelocityStatusText_ = QString::number(current_lateral_velocity_, 'f'); 
    auto current_heading_rate_ = msg->heading_rate;
    headingRateStatusText_ = QString::number(current_heading_rate_, 'f'); 
}

void GDVehicleInterfacePanel::onSteeringStatus(
  const autoware_vehicle_msgs::msg::SteeringReport::ConstSharedPtr msg)
{
    auto current_steering_tire_angle_ = msg->steering_tire_angle;
    steeringStatusText_ = QString::number(current_steering_tire_angle_, 'f', 8);  
}

void GDVehicleInterfacePanel::onHazardLightStatus(
  const autoware_vehicle_msgs::msg::HazardLightsReport::ConstSharedPtr msg)
{
    current_hazard_light_ = msg->report;
    switch (msg->report) {
        case autoware_vehicle_msgs::msg::HazardLightsReport::DISABLE:
            hazard_light_status_label_->setText("DISABLE");
            hazard_light_status_label_->setStyleSheet("background-color: white;");
            break;
        case autoware_vehicle_msgs::msg::HazardLightsReport::ENABLE:
            hazard_light_status_label_->setText("ENABLE");
            hazard_light_status_label_->setStyleSheet("background-color: red;");
            break;
    }
    last_hazard_status_time_ = clock_->now();
}

void GDVehicleInterfacePanel::onTurnLightStatus(
  const autoware_vehicle_msgs::msg::TurnIndicatorsReport::ConstSharedPtr msg)
{
  current_turn_light_ = msg->report; 
    switch (msg->report) {
        case autoware_vehicle_msgs::msg::TurnIndicatorsReport::DISABLE:
            left_light_indicator_status_label_->setText("DISABLE");
            left_light_indicator_status_label_->setStyleSheet("background-color: white;");
            right_light_indicator_status_label_->setText("DISABLE");
            right_light_indicator_status_label_->setStyleSheet("background-color: white;");
            break;
        case autoware_vehicle_msgs::msg::TurnIndicatorsReport::ENABLE_LEFT:
            left_light_indicator_status_label_->setText("ENABLE");
            left_light_indicator_status_label_->setStyleSheet("background-color: green;");
            right_light_indicator_status_label_->setText("DISABLE");
            right_light_indicator_status_label_->setStyleSheet("background-color: white;");
            last_left_light_indicator_status_time_ = clock_->now();
            break;
        case autoware_vehicle_msgs::msg::TurnIndicatorsReport::ENABLE_RIGHT:
            right_light_indicator_status_label_->setText("ENABLE");
            right_light_indicator_status_label_->setStyleSheet("background-color: green;");
            left_light_indicator_status_label_->setText("DISABLE");
            left_light_indicator_status_label_->setStyleSheet("background-color: white;");
            last_right_light_indicator_status_time_ = clock_->now();
            break;
    }
}

void GDVehicleInterfacePanel::onClickSetHazardLightButton()
{
  auto hazard_light_cmd = std::make_shared<autoware_vehicle_msgs::msg::HazardLightsCommand>();
  if (current_hazard_light_== autoware_vehicle_msgs::msg::HazardLightsReport::DISABLE){
    hazard_light_cmd->command = autoware_vehicle_msgs::msg::HazardLightsCommand::ENABLE;
  }
  else {
    hazard_light_cmd->command = autoware_vehicle_msgs::msg::HazardLightsCommand::DISABLE;
  }
  pub_hazard_light_command_->publish(*hazard_light_cmd);
}

void GDVehicleInterfacePanel::onClickSetLeftTurnButton()
{
  auto left_turn_cmd = std::make_shared<autoware_vehicle_msgs::msg::TurnIndicatorsCommand>();
  if (current_turn_light_== autoware_vehicle_msgs::msg::TurnIndicatorsReport::ENABLE_LEFT){
    left_turn_cmd->command = autoware_vehicle_msgs::msg::TurnIndicatorsCommand::DISABLE;
  }
  else {
  left_turn_cmd->command = autoware_vehicle_msgs::msg::TurnIndicatorsCommand::ENABLE_LEFT;
  }
  pub_turn_light_command_->publish(*left_turn_cmd);
}

void GDVehicleInterfacePanel::onClickSetRightTurnButton()
{
  auto right_turn_cmd = std::make_shared<autoware_vehicle_msgs::msg::TurnIndicatorsCommand>();
  if (current_turn_light_== autoware_vehicle_msgs::msg::TurnIndicatorsReport::ENABLE_RIGHT){
    right_turn_cmd->command = autoware_vehicle_msgs::msg::TurnIndicatorsCommand::DISABLE;
  }
  else {
  right_turn_cmd->command = autoware_vehicle_msgs::msg::TurnIndicatorsCommand::ENABLE_RIGHT;
  }
  pub_turn_light_command_->publish(*right_turn_cmd);
}

void GDVehicleInterfacePanel::onClickSetGearButton()
{
  auto gear_cmd = std::make_shared<autoware_vehicle_msgs::msg::GearCommand>();
  gear_cmd->command = pub_gear_cmd_input_->value();
  pub_gear_command_->publish(*gear_cmd);
}

void GDVehicleInterfacePanel::onVehicleEmergencyStatus(
  const tier4_vehicle_msgs::msg::VehicleEmergencyStamped::ConstSharedPtr msg)
{
    current_vehicle_emergency_ = msg->emergency;
    if (current_vehicle_emergency_){
      emergency_button_ptr_->setText(QString::fromStdString("Clear Emergency Command"));
      emergency_button_ptr_->setStyleSheet("background-color: red;");
      }
    else {
      emergency_button_ptr_->setText(QString::fromStdString("Send Emergency Command"));
      emergency_button_ptr_->setStyleSheet("background-color: white;");
      }
}

void GDVehicleInterfacePanel::onClickEmergencyButton()
{
  auto emergency_cmd = std::make_shared<tier4_vehicle_msgs::msg::VehicleEmergencyStamped>();
  emergency_cmd->emergency = !current_vehicle_emergency_;
  pub_vehicle_emergency_->publish(*emergency_cmd);
  
}

void GDVehicleInterfacePanel::checkMessageTimeouts()
{
    auto now = clock_->now();
    rclcpp::Duration timeout_threshold(std::chrono::seconds(1)); // 1 second timeout
    if ((now - last_battery_status_time_).seconds() > timeout_threshold.seconds()) {
        batteryLevelText_ = defaultText_;
    }
    if ((now - last_heartbeat_status_time_).seconds() > timeout_threshold.seconds()) {
        heartbeatStatusText_ = defaultText_;
    }
    if ((now - last_hazard_status_time_).seconds() > timeout_threshold.seconds()) {
        hazard_light_status_label_->setText("DISABLE");
        hazard_light_status_label_->setStyleSheet("background-color: white;");
    }
    if ((now - last_left_light_indicator_status_time_).seconds() > timeout_threshold.seconds()) {
        left_light_indicator_status_label_->setText("DISABLE");
        left_light_indicator_status_label_->setStyleSheet("background-color: white;");
    }
    if ((now - last_right_light_indicator_status_time_).seconds() > timeout_threshold.seconds()) {
        right_light_indicator_status_label_->setText("DISABLE");
        right_light_indicator_status_label_->setStyleSheet("background-color: white;");
    }
}

void GDVehicleInterfacePanel::updateLabels()
{
  battery_status_label_->setText(batteryLevelText_);
  heartbeat_label_->setText(heartbeatStatusText_);
  control_mode_label_->setText(controlModeStatusText_);
  gear_status_label_->setText(gearStatusText_);
  velocity_status_label_->setText(longVelocityStatusText_);
  steering_status_label_->setText(steeringStatusText_);
}

}
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(rviz_plugins::GDVehicleInterfacePanel, rviz_common::Panel)
