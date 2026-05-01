#include "object_detection_bridge/object_detection_zed_bridge.hpp"

#include "geometry_msgs/msg/pose.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

using std::placeholders::_1;

ObjectDetectionBridge::ObjectDetectionBridge() : Node("object_detection_zed_bridge")
{
  _tf_buffer = std::make_shared<tf2_ros::Buffer>(this->get_clock());
  _tf_listener = std::make_shared<tf2_ros::TransformListener>(*_tf_buffer);

  _subscription_zed_objects = this->create_subscription<zed_interfaces::msg::ObjectsStamped>(
    "/zed/zed_node/obj_det/objects", 10,
    std::bind(&ObjectDetectionBridge::zed_objects_callback, this, _1));

  _publisher_autoware_objects =
    this->create_publisher<autoware_perception_msgs::msg::PredictedObjects>(
      "/perception/object_recognition/objects", 10);
}

uint8_t ObjectDetectionBridge::get_autoware_object_classification(
  const std::string & zed_sublabel) const
{
  if (zed_sublabel == "Car") {
    return CAR;
  }
  if (zed_sublabel == "Truck") {
    return TRUCK;
  }
  if (zed_sublabel == "Bus") {
    return BUS;
  }
  if (zed_sublabel == "Motorbike") {
    return MOTORCYCLE;
  }
  if (zed_sublabel == "Bicycle") {
    return BICYCLE;
  }
  if (zed_sublabel == "Person") {
    return PEDESTRIAN;
  }
  return UNKNOWN;
}

void ObjectDetectionBridge::zed_objects_callback(
  const zed_interfaces::msg::ObjectsStamped::SharedPtr msg)
{
  autoware_perception_msgs::msg::PredictedObjects autoware_objects{};

  autoware_objects.header.stamp = this->get_clock()->now();
  autoware_objects.header.frame_id = "map";

  for (auto zed_object : msg->objects) {
    autoware_perception_msgs::msg::PredictedObject autoware_object{};

    auto label_id_unsigned = static_cast<uint16_t>(zed_object.label_id);
    std::array<uint8_t, 16> uuid{};
    uuid[0] = label_id_unsigned & 0xff;
    uuid[1] = (label_id_unsigned & 0xff00) >> 8;
    autoware_object.object_id.uuid = uuid;

    autoware_object.existence_probability =
      1.0f - static_cast<float>(zed_object.confidence) * 0.01f;

    autoware_perception_msgs::msg::ObjectClassification classification{};
    classification.label = get_autoware_object_classification(zed_object.sublabel);
    classification.probability = 1.0;
    autoware_object.classification.push_back(classification);

    geometry_msgs::msg::Pose pose;
    pose.position.x = zed_object.position[0];
    pose.position.y = zed_object.position[1];
    pose.position.z = zed_object.position[2];

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
    tf2::doTransform(pose, pose, base_link_to_map);

    autoware_object.kinematics.initial_pose_with_covariance.pose.position.x = pose.position.x;
    autoware_object.kinematics.initial_pose_with_covariance.pose.position.y = pose.position.y;
    autoware_object.kinematics.initial_pose_with_covariance.pose.position.z = pose.position.z;

    autoware_object.shape.type = 0;
    autoware_object.shape.dimensions.x = zed_object.dimensions_3d[0];
    autoware_object.shape.dimensions.y = zed_object.dimensions_3d[2];  // y and z are swapped
    autoware_object.shape.dimensions.z = zed_object.dimensions_3d[1];  // y and z are swapped

    autoware_objects.objects.push_back(autoware_object);
  }

  _publisher_autoware_objects->publish(autoware_objects);
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ObjectDetectionBridge>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}