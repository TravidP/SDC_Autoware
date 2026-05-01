#include <autoware_adapi_v1_msgs/msg/operation_mode_state.hpp>
#include <chrono>
#include <memory>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <rclcpp/rclcpp.hpp>

class GridPublisher : public rclcpp::Node
{
public:
  GridPublisher() : Node("mock_grid_publisher")
  {
    timer_ = create_wall_timer(
      std::chrono::milliseconds(2000), std::bind(&GridPublisher::timerCallback, this));
    grid_pub_ =
      create_publisher<nav_msgs::msg::OccupancyGrid>("/perception/occupancy_grid_map/map", 1);
  }

private:
  void timerCallback()
  {
    nav_msgs::msg::OccupancyGrid msg;
    msg.header.stamp = get_clock()->now();
    msg.header.frame_id = "map";
    msg.info.resolution = 0.5;
    msg.info.width = 10;
    msg.info.height = 10;
    msg.info.origin.position.x = 0.0;
    msg.info.origin.position.y = 0.0;
    std::vector<int8_t> data(100, 0);
    msg.data = data;

    grid_pub_->publish(msg);

    return;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr grid_pub_;
};

class OpModePublisher : public rclcpp::Node
{
public:
  OpModePublisher() : Node("mock_operation_mode_publisher")
  {
    timer_ = create_wall_timer(
      std::chrono::milliseconds(2000), std::bind(&OpModePublisher::timerCallback, this));
    rclcpp::QoS qos_profile(1);
    qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);
    qos_profile.durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);
    qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);
    opmode_pub_ = create_publisher<autoware_adapi_v1_msgs::msg::OperationModeState>(
      "/system/operation_mode/state", qos_profile);
  }

private:
  void timerCallback()
  {
    autoware_adapi_v1_msgs::msg::OperationModeState msg;
    msg.mode = 2;  //Autonomous
    msg.is_autoware_control_enabled = true;
    msg.is_in_transition = false;
    msg.is_stop_mode_available = true;
    msg.is_autonomous_mode_available = true;
    msg.is_local_mode_available = true;
    msg.is_remote_mode_available = true;
    opmode_pub_->publish(msg);
    return;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<autoware_adapi_v1_msgs::msg::OperationModeState>::SharedPtr opmode_pub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  if (argc == 3) {
    auto op_node = std::make_shared<OpModePublisher>();

    rclcpp::spin(op_node);
    rclcpp::shutdown();
    return 0;

  } else {
    auto grid_node = std::make_shared<GridPublisher>();
    auto op_node = std::make_shared<OpModePublisher>();

    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(grid_node);
    executor.add_node(op_node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
  }
}
