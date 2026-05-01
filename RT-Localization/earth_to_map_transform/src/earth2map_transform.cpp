#include "earth_to_map_transform/earth2map_transform.hpp"

#include <GeographicLib/Geocentric.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>

using std::placeholders::_1;

Earth2MapTransform::Earth2MapTransform() : Node("earth2map_transform")
{
  lookup_timeout_ = this->declare_parameter<double>("lookup_timeout", 5.0);
  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
  broadcaster_ = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);
  rclcpp::QoS qos_profile(1);                                         // Depth set to 1
  qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);       // Best Effort reliability
  qos_profile.durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);  // Volatile durability
  qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);              // Keep last history
  map_proj_info_subs_ = this->create_subscription<autoware_map_msgs::msg::MapProjectorInfo>(
    "/map/map_projector_info", qos_profile,
    std::bind(&Earth2MapTransform::map_proj_info_callback, this, _1));
}

void Earth2MapTransform::wgs84ToEcef(
  double latitude, double longitude, double altitude, double & x, double & y, double & z)
{
  GeographicLib::Geocentric earth(
    GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f());

  earth.Forward(latitude, longitude, altitude, x, y, z);
}

void Earth2MapTransform::map_proj_info_callback(
  const autoware_map_msgs::msg::MapProjectorInfo::SharedPtr msg)
{
  map_proj_info_ = msg->map_origin;
  double map_ecef_x, map_ecef_y, map_ecef_z;
  Earth2MapTransform::wgs84ToEcef(
    map_proj_info_.latitude, map_proj_info_.longitude, map_proj_info_.altitude, map_ecef_x,
    map_ecef_y, map_ecef_z);

  try {
    ecef2enu0_transform_ = tf_buffer_->lookupTransform(
      "FP_ECEF", "FP_ENU0", tf2::TimePointZero, tf2::durationFromSec(lookup_timeout_));
  } catch (tf2::TransformException & ex) {
    RCLCPP_ERROR(get_logger(), "Transform exception: %s", ex.what());
    return;
  }

  ecefmap_transform_.header.frame_id = "FP_ECEF";
  ecefmap_transform_.child_frame_id = "map";
  ecefmap_transform_.transform.translation.x = map_ecef_x;
  ecefmap_transform_.transform.translation.y = map_ecef_y;
  ecefmap_transform_.transform.translation.z = map_ecef_z;
  ecefmap_transform_.transform.rotation = ecef2enu0_transform_.transform.rotation;

  broadcaster_->sendTransform(ecefmap_transform_);
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<Earth2MapTransform>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}