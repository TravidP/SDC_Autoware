#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"  // Change to your desired type

using std::placeholders::_1;

class QoSRepublisher : public rclcpp::Node
{
public:
  QoSRepublisher()
  : Node("qos_republisher")
  {
    // Declare parameters
    input_topic_ = this->declare_parameter<std::string>("input_topic", "input_topic");
    output_topic_ = this->declare_parameter<std::string>("output_topic", "output_topic");

    // Input QoS parameters
    auto in_reliability = this->declare_parameter<std::string>("input_qos.reliability", "reliable");
    auto in_durability  = this->declare_parameter<std::string>("input_qos.durability", "volatile");
    auto in_history     = this->declare_parameter<std::string>("input_qos.history", "keep_last");
    int in_depth        = this->declare_parameter<int>("input_qos.depth", 10);

    // Output QoS parameters
    auto out_reliability = this->declare_parameter<std::string>("output_qos.reliability", "reliable");
    auto out_durability  = this->declare_parameter<std::string>("output_qos.durability", "volatile");
    auto out_history     = this->declare_parameter<std::string>("output_qos.history", "keep_last");
    int out_depth        = this->declare_parameter<int>("output_qos.depth", 10);

    // Build QoS profiles
    rclcpp::QoS input_qos = make_qos(in_reliability, in_durability, in_history, in_depth);
    rclcpp::QoS output_qos = make_qos(out_reliability, out_durability, out_history, out_depth);

    // Publisher and subscriber
    pub_ = this->create_publisher<nav_msgs::msg::Odometry>(output_topic_, output_qos);
    sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
      input_topic_,
      input_qos,
      std::bind(&QoSRepublisher::callback, this, _1)
    );

    RCLCPP_INFO(this->get_logger(), "QoS Republisher started: '%s' -> '%s'", input_topic_.c_str(), output_topic_.c_str());
  }

private:
  void callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    pub_->publish(*msg);
  }

  rclcpp::QoS make_qos(const std::string & reliability,
                       const std::string & durability,
                       const std::string & history,
                       int depth)
  {
    rclcpp::QoS qos(depth);

    if (reliability == "best_effort")
      qos.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
    else
      qos.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);

    if (durability == "transient_local")
      qos.durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);
    else
      qos.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);

    if (history == "keep_all")
      qos.keep_all();
    else
      qos.keep_last(depth);

    return qos;
  }

  std::string input_topic_;
  std::string output_topic_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr pub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<QoSRepublisher>());
  rclcpp::shutdown();
  return 0;
}
