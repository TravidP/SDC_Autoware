#ifndef OBJECT_DETECTION_ZED_BRIDGE_HPP
#define OBJECT_DETECTION_ZED_BRIDGE_HPP

#include <tf2_ros/transform_listener.h>

#include <memory>

#include "autoware_perception_msgs/msg/predicted_objects.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/buffer.h"
#include "zed_interfaces/msg/objects_stamped.hpp"

class ObjectDetectionBridge : public rclcpp::Node
{
public:
  ObjectDetectionBridge();

private:
  // Autoware object classifications:
  const uint8_t UNKNOWN = 0;
  const uint8_t CAR = 1;
  const uint8_t TRUCK = 2;
  const uint8_t BUS = 3;
  // const uint8_t TRAILER = 4;
  const uint8_t MOTORCYCLE = 5;
  const uint8_t BICYCLE = 6;
  const uint8_t PEDESTRIAN = 7;
  // source:
  // https://gitlab.com/autowarefoundation/autoware.auto/autoware_auto_msgs/-/blob/master/autoware_auto_perception_msgs/msg/ObjectClassification.idl

  uint8_t get_autoware_object_classification(const std::string & zed_sublabel) const;

  std::shared_ptr<tf2_ros::Buffer> _tf_buffer;
  std::shared_ptr<tf2_ros::TransformListener> _tf_listener;

  void zed_objects_callback(const zed_interfaces::msg::ObjectsStamped::SharedPtr msg);
  rclcpp::Subscription<zed_interfaces::msg::ObjectsStamped>::SharedPtr _subscription_zed_objects;
  rclcpp::Publisher<autoware_perception_msgs::msg::PredictedObjects>::SharedPtr
    _publisher_autoware_objects;
};

#endif  // OBJECT_DETECTION_BRIDGE_HPP