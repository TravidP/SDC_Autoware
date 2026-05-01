#ifndef GOAL_PUBLISHER_HPP
#define GOAL_PUBLISHER_HPP


#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <nav_msgs/msg/odometry.hpp>

#include <autoware_adapi_v1_msgs/msg/route_state.hpp>
#include <autoware_internal_msgs/msg/mission_remaining_distance_time.hpp>
#include <autoware_planning_msgs/msg/lanelet_route.hpp>
#include <tier4_planning_msgs/msg/reroute_availability.hpp>

#include <autoware_adapi_v1_msgs/srv/clear_route.hpp>
#include <autoware_adapi_v1_msgs/srv/set_route_points.hpp>

class GoalPublisher : public rclcpp::Node {
    public:
        GoalPublisher();

    private:
        float distance_to_goal_;
        float time_to_goal_;
        std::size_t goal_number_ = 1;
        bool first_execution_ = false;
        bool reroute_available_ = false;
        double distance_to_goal_upper_limit_ = 12.0;

        std::string filename_;
        std::vector<geometry_msgs::msg::Pose> goal_list_;
        nav_msgs::msg::Odometry::SharedPtr vehicle_pose_;
        autoware_adapi_v1_msgs::msg::RouteState::SharedPtr route_state_;
        autoware_planning_msgs::msg::LaneletRoute::SharedPtr route_;

        void localization_callback(const nav_msgs::msg::Odometry::SharedPtr msg);
        void route_state_callback(const autoware_adapi_v1_msgs::msg::RouteState::SharedPtr msg);
        void route_callback(const autoware_planning_msgs::msg::LaneletRoute::SharedPtr msg);
        void path_distance_time_callback(const autoware_internal_msgs::msg::MissionRemainingDistanceTime::SharedPtr msg);
        void reroute_callback(const tier4_planning_msgs::msg::RerouteAvailability::SharedPtr msg);

        rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr vehicle_position_subs_;
        rclcpp::Subscription<autoware_adapi_v1_msgs::msg::RouteState>::SharedPtr route_state_subs_;
        rclcpp::Subscription<autoware_planning_msgs::msg::LaneletRoute>::SharedPtr route_subs_;
        rclcpp::Subscription<autoware_internal_msgs::msg::MissionRemainingDistanceTime>::SharedPtr path_distance_time_subs_;
        rclcpp::Subscription<tier4_planning_msgs::msg::RerouteAvailability>::SharedPtr reroute_availability_subs_;

        rclcpp::Client<autoware_adapi_v1_msgs::srv::SetRoutePoints>::SharedPtr set_route_client_;
        rclcpp::Client<autoware_adapi_v1_msgs::srv::SetRoutePoints>::SharedPtr change_route_client_;
        rclcpp::Client<autoware_adapi_v1_msgs::srv::ClearRoute>::SharedPtr clear_route_client_;

        void load_goals_from_yaml(const std::string &filename);       

        template<typename ServiceType, typename RequestType>
        void call_service(
            typename rclcpp::Client<ServiceType>::SharedPtr client,
            typename RequestType::SharedPtr request,
            int max_attempts,
            int current_attempt
        );
};

// TODO: Replace with a simpler algorithm if possible
template<typename ServiceType, typename RequestType>
void GoalPublisher::call_service(
    typename rclcpp::Client<ServiceType>::SharedPtr client,
    typename RequestType::SharedPtr request,
    int max_attempts,
    int current_attempt) {
        // Check if the client initialize
        if (!client) {
            RCLCPP_ERROR_STREAM(this->get_logger(), "Client is not initialized.");
            rclcpp::shutdown();
        }
        // Check if the service available
        if (!client->wait_for_service(std::chrono::seconds(5))) {
            RCLCPP_ERROR_STREAM(this->get_logger(), "Service " << client->get_service_name() << " is not available.");
            rclcpp::shutdown();
        }

        RCLCPP_INFO(this->get_logger(), "Calling service...");

        // Retries the service call up to max_attempts times if the call fails or times out.
        auto result = client->async_send_request(request, [this, client, request, max_attempts, current_attempt](typename rclcpp::Client<ServiceType>::SharedFuture future) {
            if (future.wait_for(std::chrono::seconds(4)) == std::future_status::ready) {
                auto response = future.get();
                if (response->status.success) {
                    RCLCPP_INFO_STREAM(this->get_logger(), "Service " << client->get_service_name() << " call succeeded");
                } else {
                    RCLCPP_ERROR_STREAM(this->get_logger(), "Service " << client->get_service_name() << " call failed");
                    if (current_attempt < max_attempts - 1) {
                        std::chrono::milliseconds delay(1000);
                        std::this_thread::sleep_for(delay);
                        call_service<ServiceType, RequestType>(client, request, max_attempts, current_attempt + 1);
                    } else {
                        RCLCPP_ERROR_STREAM(this->get_logger(), "Max attempts reached. Service " << client->get_service_name() << " call failed after " << max_attempts << " attempts");
                        rclcpp::shutdown();
                    }
                }
            } 
            else {
                RCLCPP_ERROR_STREAM(this->get_logger(), "Service " << client->get_service_name() << " call timed out");
                if (current_attempt < max_attempts - 1) {
                    std::chrono::milliseconds delay(1000);
                    std::this_thread::sleep_for(delay);
                    call_service<ServiceType, RequestType>(client, request, max_attempts, current_attempt + 1);
                } else {
                    RCLCPP_ERROR_STREAM(this->get_logger(), "Max attempts reached. Service " << client->get_service_name() << " call timed out after " << max_attempts << " attempts");
                    rclcpp::shutdown();
                }
            }
        });
}



#endif // GOAL_PUBLISHER_HPP