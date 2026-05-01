#include <tf2_ros/transform_listener.h>

#include <chrono>
#include <memory>

#include "autoware_perception_msgs/msg/predicted_objects.hpp"
#include "continental_msgs/msg/continental_ars548_object_list.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/buffer.h"

using std::placeholders::_1;
using namespace std::chrono_literals;
using Classification = autoware_perception_msgs::msg::ObjectClassification;

class RadarConverter : public rclcpp::Node
{
public:
  RadarConverter() : Node("rt_radar_converter")
  {
    use_fixed_object_width_ = declare_parameter<bool>("use_fixed_object_width", true);
    fixed_object_width_ = declare_parameter<double>("fixed_object_width", 0.25);
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    continental_objects_subscription_ =
      this->create_subscription<continental_msgs::msg::ContinentalArs548ObjectList>(
        "~/input/continental_objects", rclcpp::SensorDataQoS(),
        std::bind(&RadarConverter::continental_objects_callback, this, _1));
    autoware_objects_publisher_ =
      this->create_publisher<autoware_perception_msgs::msg::PredictedObjects>(
        "~/output/predicted_objects", 10);

    timer_ = this->create_wall_timer(5000ms, std::bind(&RadarConverter::timer_callback, this));
  }

private:
  void timer_callback()
  {
    if (!radar_message_received_) {
      RCLCPP_WARN_STREAM(this->get_logger(), "[Radar converter] Waiting for radar messages");
    }
    radar_message_received_ = false;
  }

  static void add_classification(
    std::vector<autoware_perception_msgs::msg::ObjectClassification> & classification,
    uint8_t label, float probability)
  {
    // only keep the classification with the highest probability
    // TODO: keep all classifications? (they may need to be sorted by descending probability)
    if (classification.empty()) {
      autoware_perception_msgs::msg::ObjectClassification classification_item{};
      classification_item.label = label;
      classification_item.probability = probability;
      classification.push_back(classification_item);
    } else if (classification[0].probability < probability) {
      classification[0].label = label;
      classification[0].probability = probability;
    }
  }

  std::vector<autoware_perception_msgs::msg::ObjectClassification>
  get_autoware_object_classification(const continental_msgs::msg::ContinentalArs548Object object)
  {
    std::vector<autoware_perception_msgs::msg::ObjectClassification> classification{};

    add_classification(classification, Classification::UNKNOWN, 0.01f);

    add_classification(
      classification, Classification::BICYCLE, object.raw_classification_bicycle * 0.01f);
    add_classification(classification, Classification::CAR, object.raw_classification_car * 0.01f);
    add_classification(
      classification, Classification::MOTORCYCLE, object.raw_classification_motorcycle * 0.01f);
    add_classification(
      classification, Classification::PEDESTRIAN, object.raw_classification_pedestrian * 0.01f);
    add_classification(classification, Classification::TRUCK, object.raw_classification_truck * 0.01f);

    return classification;
  }

  // source: see comment below for get_pose()
  constexpr static int REFERENCE_POINTS_NUM = 9;
  constexpr static std::array<std::array<double, 2>, REFERENCE_POINTS_NUM> reference_to_center_ = {
    {{{-1.0, -1.0}},
     {{-1.0, 0.0}},
     {{-1.0, 1.0}},
     {{0.0, 1.0}},
     {{1.0, 1.0}},
     {{1.0, 0.0}},
     {{1.0, -1.0}},
     {{0.0, -1.0}},
     {{0.0, 0.0}}}};

  // the computations in this function were taken from the ConvertToMarkers() function
  // from continental_ars548_decoder_ros_wrapper.cpp of the nebula_ros project
  geometry_msgs::msg::Pose get_pose(const continental_msgs::msg::ContinentalArs548Object object)
  {
    const double half_length = 0.5 * object.shape_length_edge_mean;
    const double half_width = 0.5 * object.shape_width_edge_mean;
    const int reference_index = std::min<int>(object.position_reference, 8);
    const double & yaw = object.orientation;

    geometry_msgs::msg::Pose pose;
    pose.position.x = object.position.x +
                      std::cos(yaw) * half_length * reference_to_center_[reference_index][0] -
                      std::sin(yaw) * half_width * reference_to_center_[reference_index][1];
    pose.position.y = object.position.y +
                      std::sin(yaw) * half_length * reference_to_center_[reference_index][0] +
                      std::cos(yaw) * half_width * reference_to_center_[reference_index][1];
    pose.position.z = object.position.z;
    pose.orientation.w = std::cos(0.5 * yaw);
    pose.orientation.z = std::sin(0.5 * yaw);

    return pose;
  }

  void continental_objects_callback(const continental_msgs::msg::ContinentalArs548ObjectList & msg)
  {
    radar_message_received_ = true;

    autoware_perception_msgs::msg::PredictedObjects autoware_objects{};
    autoware_objects.header.stamp = this->get_clock()->now();
    autoware_objects.header.frame_id = "map";

    geometry_msgs::msg::TransformStamped base_link_to_map;
    auto source_frame = "base_link";
    auto target_frame = "map";
    try {
      base_link_to_map =
        tf_buffer_->lookupTransform(target_frame, source_frame, tf2::TimePointZero);
    } catch (const tf2::TransformException & ex) {
      RCLCPP_INFO(
        this->get_logger(), "Could not transform %s to %s: %s", source_frame, target_frame,
        ex.what());
      return;
    }

    for (auto & continental_object : msg.objects) {
      autoware_perception_msgs::msg::PredictedObject autoware_object{};

      uint32_t id = continental_object.object_id;
      std::array<uint8_t, 16> uuid{};
      uuid[0] = id & 0xff;
      uuid[1] = (id & 0xff00) >> 8;
      uuid[2] = (id & 0xff0000) >> 16;
      uuid[3] = (id & 0xff000000) >> 24;
      autoware_object.object_id.uuid = uuid;

      autoware_object.existence_probability = continental_object.raw_existence_probability;

      autoware_object.classification = get_autoware_object_classification(continental_object);

      auto pose = get_pose(continental_object);
      tf2::doTransform(pose, pose, base_link_to_map);

      autoware_object.kinematics.initial_pose_with_covariance.pose = pose;

      autoware_object.shape.type = autoware_perception_msgs::msg::Shape::BOUNDING_BOX;

      if (use_fixed_object_width_) {
        autoware_object.shape.dimensions.x = fixed_object_width_;
        autoware_object.shape.dimensions.y = fixed_object_width_;
      } else {
        autoware_object.shape.dimensions.x = continental_object.shape_length_edge_mean;
        autoware_object.shape.dimensions.y = continental_object.shape_width_edge_mean;
      }
      autoware_object.shape.dimensions.z = 2.0;

      autoware_objects.objects.push_back(autoware_object);
    }

    autoware_objects_publisher_->publish(autoware_objects);
  }

  bool use_fixed_object_width_;
  double fixed_object_width_;

  rclcpp::TimerBase::SharedPtr timer_;
  bool radar_message_received_ = false;

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

  rclcpp::Subscription<continental_msgs::msg::ContinentalArs548ObjectList>::SharedPtr
    continental_objects_subscription_;
  rclcpp::Publisher<autoware_perception_msgs::msg::PredictedObjects>::SharedPtr
    autoware_objects_publisher_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RadarConverter>());
  rclcpp::shutdown();
  return 0;
}