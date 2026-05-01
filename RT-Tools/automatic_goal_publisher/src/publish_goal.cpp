#include <fstream>
#include <chrono>
#include <cmath>
#include <vector>

#include "yaml-cpp/yaml.h"
#include "automatic_goal_publisher/publish_goal.hpp"

using std::placeholders::_1;

GoalPublisher::GoalPublisher() : Node("goal_publisher") {
    filename_ = declare_parameter<std::string>("filename", "automatic_goal_publisher/config/goal_list.yaml");
    distance_to_goal_upper_limit_ = declare_parameter<float>("distance_to_goal_upper_limit", 20.0);

    // Create list of goal from the yaml file
    load_goals_from_yaml(filename_);

    set_route_client_ = this->create_client<autoware_adapi_v1_msgs::srv::SetRoutePoints>(
        "/api/routing/set_route_points");
    change_route_client_ = this->create_client<autoware_adapi_v1_msgs::srv::SetRoutePoints>(
        "/api/routing/change_route_points");
    clear_route_client_ = this->create_client<autoware_adapi_v1_msgs::srv::ClearRoute>(
        "/api/routing/clear_route");

    rclcpp::QoS qos_profile(1);
    qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
    qos_profile.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);
    qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);
    vehicle_position_subs_ = this->create_subscription<nav_msgs::msg::Odometry>(
        "/localization/kinematic_state", qos_profile, std::bind(&GoalPublisher::localization_callback, this, _1));
    route_state_subs_ = this->create_subscription<autoware_adapi_v1_msgs::msg::RouteState>(
        "/api/routing/state", qos_profile, std::bind(&GoalPublisher::route_state_callback, this, _1));
    route_subs_ = this->create_subscription<autoware_planning_msgs::msg::LaneletRoute>(
        "/planning/mission_planning/route", qos_profile, std::bind(&GoalPublisher::route_callback, this, _1));
    path_distance_time_subs_ = this->create_subscription<autoware_internal_msgs::msg::MissionRemainingDistanceTime>(
        "/planning/mission_remaining_distance_time", qos_profile, std::bind(&GoalPublisher::path_distance_time_callback, this, _1));
    reroute_availability_subs_ = this->create_subscription<tier4_planning_msgs::msg::RerouteAvailability>(
        "/planning/scenario_planning/lane_driving/behavior_planning/behavior_path_planner/output/is_reroute_available", 10, std::bind(&GoalPublisher::reroute_callback, this, _1));

}

// Function to load goals from a YAML file
void GoalPublisher::load_goals_from_yaml(const std::string &filename)
{
  std::vector<geometry_msgs::msg::Pose> goals;
  try
  {
    YAML::Node config = YAML::LoadFile(filename);
    for (const auto &goal : config["goals"])
    {
        geometry_msgs::msg::Pose goal_pose;
        goal_pose.position.x = goal["position"]["x"].as<double>();
        goal_pose.position.y = goal["position"]["y"].as<double>();
        goal_pose.position.z = goal["position"]["z"].as<double>();
        goal_pose.orientation.x = goal["orientation"]["x"].as<double>();
        goal_pose.orientation.y = goal["orientation"]["y"].as<double>();
        goal_pose.orientation.z = goal["orientation"]["z"].as<double>();
        goal_pose.orientation.w = goal["orientation"]["w"].as<double>();
        goal_list_.push_back(goal_pose);
    }
  }
  catch (const YAML::Exception &e)
  {
    throw std::runtime_error("YAML Exception: " + std::string(e.what()));
  }
}


void GoalPublisher::path_distance_time_callback(const autoware_internal_msgs::msg::MissionRemainingDistanceTime::SharedPtr msg) {
    distance_to_goal_ = msg->remaining_distance;
    time_to_goal_ = msg->remaining_time;
    // Check the remaining goal and first execution condition
    if (goal_number_ < goal_list_.size() && first_execution_) {
        // If the route message not empty and route state is set, then call change the route service.
        // Otherwise call set the route service.
        if (route_ && route_state_->state == autoware_adapi_v1_msgs::msg::RouteState::SET){
            RCLCPP_INFO_STREAM_THROTTLE(get_logger(), *get_clock(), 10000, "Remaining distance to goal " << distance_to_goal_ << ", goal number " << goal_number_ << ", goal size " << goal_list_.size());
            if (reroute_available_ && abs(distance_to_goal_) > 0.5  && abs(distance_to_goal_) <= distance_to_goal_upper_limit_) {
                auto _request_service = std::make_shared<autoware_adapi_v1_msgs::srv::SetRoutePoints::Request>();
                _request_service->header.stamp =  this->get_clock()->now();
                _request_service->header.frame_id = "map";
                _request_service->goal = goal_list_[goal_number_];
                call_service<autoware_adapi_v1_msgs::srv::SetRoutePoints, autoware_adapi_v1_msgs::srv::SetRoutePoints::Request>(change_route_client_, _request_service, 10, 0);
                goal_number_ ++;
            }
        } else {
            auto _clear_route_service = std::make_shared<autoware_adapi_v1_msgs::srv::ClearRoute::Request>();
            call_service<autoware_adapi_v1_msgs::srv::ClearRoute, autoware_adapi_v1_msgs::srv::ClearRoute::Request>(clear_route_client_, _clear_route_service, 10, 0);
            auto _request_service = std::make_shared<autoware_adapi_v1_msgs::srv::SetRoutePoints::Request>();
            _request_service->header.stamp =  this->get_clock()->now();
            _request_service->header.frame_id = "map";
            _request_service->goal = goal_list_[goal_number_];
            call_service<autoware_adapi_v1_msgs::srv::SetRoutePoints, autoware_adapi_v1_msgs::srv::SetRoutePoints::Request>(set_route_client_, _request_service, 10, 0);
            goal_number_ ++;

        }

    }
}

// Send the first goal if vehicle localize
void GoalPublisher::localization_callback(const nav_msgs::msg::Odometry::SharedPtr msg) {
    vehicle_pose_ = msg;
    if (!first_execution_) {
        auto _clear_route_service = std::make_shared<autoware_adapi_v1_msgs::srv::ClearRoute::Request>();
        call_service<autoware_adapi_v1_msgs::srv::ClearRoute, autoware_adapi_v1_msgs::srv::ClearRoute::Request>(clear_route_client_, _clear_route_service, 10, 0);
        RCLCPP_INFO_STREAM(get_logger(), "Route is cleared");

        auto _request_service = std::make_shared<autoware_adapi_v1_msgs::srv::SetRoutePoints::Request>();
        _request_service->header.stamp =  this->get_clock()->now();
        _request_service->header.frame_id = "map";
        _request_service->goal = goal_list_[0];
        call_service<autoware_adapi_v1_msgs::srv::SetRoutePoints, autoware_adapi_v1_msgs::srv::SetRoutePoints::Request>(set_route_client_, _request_service, 10, 0);
        first_execution_ = true;
        RCLCPP_INFO_STREAM(get_logger(), "First goal is executed");
    }
}

void GoalPublisher::route_state_callback(const autoware_adapi_v1_msgs::msg::RouteState::SharedPtr msg){
    route_state_ = msg;
    auto route_string = "";
    if (route_state_->state == autoware_adapi_v1_msgs::msg::RouteState::UNSET){
        route_string = "1 (UNSET)";
    }
    else if (route_state_->state == autoware_adapi_v1_msgs::msg::RouteState::SET) {
        route_string = "2 (SET)";
    }
    else if (route_state_->state == autoware_adapi_v1_msgs::msg::RouteState::ARRIVED) {
        route_string = "3 (ARRIVED)";
    }
    else if (route_state_->state == autoware_adapi_v1_msgs::msg::RouteState::CHANGING) {
        route_string = "4 (CHANGING)";
    }
    else {
        route_string = "0 (UNKNOWN)";
    }
    RCLCPP_INFO_STREAM(get_logger(), "Route state: " << route_string);

}

void GoalPublisher::route_callback(const autoware_planning_msgs::msg::LaneletRoute::SharedPtr msg) {
    route_ = msg;
    RCLCPP_INFO_STREAM(get_logger(), "Route recieved");
}

void GoalPublisher::reroute_callback(const tier4_planning_msgs::msg::RerouteAvailability::SharedPtr msg) {
    reroute_available_ = msg->availability;
}


int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<GoalPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}