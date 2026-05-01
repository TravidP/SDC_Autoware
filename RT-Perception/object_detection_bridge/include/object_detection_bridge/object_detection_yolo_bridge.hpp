
#ifndef OBJECT_DETECTION_BRIDGE_HPP
#define OBJECT_DETECTION_BRIDGE_HPP

#include <tf2_ros/transform_listener.h>

#include <chrono>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "autoware_perception_msgs/msg/predicted_objects.hpp"
#include "autoware_perception_msgs/msg/predicted_path.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/buffer.h"
#include "yolov8_msgs/msg/detection_array.hpp"
class ObjectDetectionYOLOBridge : public rclcpp::Node
{
public:
  ObjectDetectionYOLOBridge();

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
  uint8_t get_autoware_object_classification(const std::string & yolo_sublabel) const;

  // void remove_old_objects();
  void timer_callback();
  rclcpp::TimerBase::SharedPtr _timer;
  std::shared_ptr<tf2_ros::Buffer> _tf_buffer;
  std::shared_ptr<tf2_ros::TransformListener> _tf_listener;

  autoware_perception_msgs::msg::PredictedObjects autoware_objects_;
  yolov8_msgs::msg::DetectionArray::SharedPtr _last_yolo_msg;
  void yolo_objects_callback(const yolov8_msgs::msg::DetectionArray::SharedPtr msg);
  rclcpp::Subscription<yolov8_msgs::msg::DetectionArray>::SharedPtr _subscription_yolo_objects;
  rclcpp::Publisher<autoware_perception_msgs::msg::PredictedObjects>::SharedPtr
    _publisher_autoware_objects;

  // struct TrackedObject
  // {
  //     autoware_perception_msgs::msg::PredictedObject object;
  //     rclcpp::Time timestamp;
  // };
  // std::map<std::string, TrackedObject> _tracked_objects;
};

#endif  // OBJECT_DETECTION_BRIDGE_HPP
