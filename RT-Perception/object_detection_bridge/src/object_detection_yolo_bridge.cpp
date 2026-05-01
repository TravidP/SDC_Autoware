#include "object_detection_bridge/object_detection_yolo_bridge.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "geometry_msgs/msg/pose.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

using std::placeholders::_1;

ObjectDetectionYOLOBridge::ObjectDetectionYOLOBridge() : Node("object_detection_yolo_bridge")
{
  _tf_buffer = std::make_shared<tf2_ros::Buffer>(this->get_clock());
  _tf_listener = std::make_shared<tf2_ros::TransformListener>(*_tf_buffer);

  _subscription_yolo_objects = this->create_subscription<yolov8_msgs::msg::DetectionArray>(
    "/yolo/detections_3d", 10,
    std::bind(&ObjectDetectionYOLOBridge::yolo_objects_callback, this, _1));
  _publisher_autoware_objects =
    this->create_publisher<autoware_perception_msgs::msg::PredictedObjects>(
      "/perception/object_recognition/objects", 10);

  _timer = this->create_wall_timer(
    std::chrono::milliseconds(20), std::bind(&ObjectDetectionYOLOBridge::timer_callback, this));
}

uint8_t ObjectDetectionYOLOBridge::get_autoware_object_classification(
  const std::string & yolo_sublabel) const
{
  if (yolo_sublabel == "2") {
    return CAR;
  }
  if (yolo_sublabel == "7") {
    return CAR;
  }
  if (yolo_sublabel == "5") {
    return BUS;
  }
  if (yolo_sublabel == "1") {
    return BICYCLE;
  }
  if (yolo_sublabel == "0") {
    return PEDESTRIAN;
  }
  return UNKNOWN;
}

void ObjectDetectionYOLOBridge::yolo_objects_callback(
  const yolov8_msgs::msg::DetectionArray::SharedPtr msg)
{
  rclcpp::Time now = this->get_clock()->now();

  autoware_objects_.objects.clear();
  autoware_objects_.header.stamp = now;
  autoware_objects_.header.frame_id = "map";

  for (const auto & yolo_object : msg->detections) {
    autoware_perception_msgs::msg::PredictedObject autoware_object{};

    auto label_id_unsigned = static_cast<uint16_t>(std::stoi(yolo_object.id));
    std::array<uint8_t, 16> uuid{};
    uuid[0] = label_id_unsigned & 0xff;
    uuid[1] = (label_id_unsigned & 0xff00) >> 8;
    autoware_object.object_id.uuid = uuid;

    autoware_object.existence_probability = 1.0f;

    autoware_perception_msgs::msg::ObjectClassification classification{};
    classification.label = get_autoware_object_classification(yolo_object.class_name);
    classification.probability = 1.0;
    autoware_object.classification.push_back(classification);

    geometry_msgs::msg::Pose pose;
    pose.position = yolo_object.bbox3d.center.position;

    geometry_msgs::msg::TransformStamped base_link_to_map;
    auto source_frame = "base_link";
    auto target_frame = "map";

    try {
      base_link_to_map =
        _tf_buffer->lookupTransform(target_frame, source_frame, tf2::TimePointZero);
    } catch (const tf2::TransformException & ex) {
      RCLCPP_INFO(
        this->get_logger(), "Could not transform %s to %s: %s", source_frame, target_frame,
        ex.what());
      return;
    }
    tf2::doTransform(pose, pose,
                     base_link_to_map);  // This can also throw I believe

    autoware_object.kinematics.initial_pose_with_covariance.pose.position.x = pose.position.x;
    autoware_object.kinematics.initial_pose_with_covariance.pose.position.y = pose.position.y;
    autoware_object.kinematics.initial_pose_with_covariance.pose.position.z = pose.position.z;

    autoware_object.shape.type = 0;
    autoware_object.shape.dimensions.x =
      std::min(yolo_object.bbox3d.size.x, 2.0);  // yolo_object.bbox3d.size.x;
    autoware_object.shape.dimensions.y =
      std::min(yolo_object.bbox3d.size.y, 2.0);  // yolo_object.bbox3d.size.y;
    autoware_object.shape.dimensions.z =
      std::min(yolo_object.bbox3d.size.z, 2.0);  // yolo_object.bbox3d.size.z;

    // Predicted paths
    autoware_perception_msgs::msg::PredictedPath path;
    path.confidence = 1.0;
    path.time_step.sec = 0;
    path.time_step.nanosec = 500000000;

    for (int i = 0; i < 20; ++i) {
      geometry_msgs::msg::Pose predicted_pose;
      predicted_pose = autoware_object.kinematics.initial_pose_with_covariance.pose;
      path.path.push_back(predicted_pose);
    }
    autoware_object.kinematics.predicted_paths.push_back(path);

    // // Convert UUID to string for map key
    // std::string uuid_key = yolo_object.id;

    // // Check if object with the same UUID already exists
    // if (_tracked_objects.find(uuid_key) == _tracked_objects.end()) {
    //     _tracked_objects[uuid_key] = {autoware_object, now};
    // } else {
    //     // Update the timestamp and object data of the existing object
    //     _tracked_objects[uuid_key].timestamp = now;
    //     _tracked_objects[uuid_key].object = autoware_object;
    // }
    autoware_objects_.objects.push_back(autoware_object);
  }

  // // Remove objects older than 5 seconds
  // remove_old_objects();

  // // Prepare the message to be published

  // for (const auto &tracked_object : _tracked_objects) {
  //     autoware_objects_.objects.push_back(tracked_object.second.object);
  // }
}

void ObjectDetectionYOLOBridge::timer_callback()
{
  _publisher_autoware_objects->publish(autoware_objects_);
}

// void ObjectDetectionYOLOBridge::remove_old_objects()
// {
//     rclcpp::Time now = this->get_clock()->now();
//     for (auto it = _tracked_objects.begin(); it != _tracked_objects.end(); )
//     {
//         if ((now - it->second.timestamp).seconds() > 5.0) {
//             it = _tracked_objects.erase(it);
//         } else {
//             ++it;
//         }
//     }
// }

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ObjectDetectionYOLOBridge>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
