#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>

#include <GeographicLib/Geocentric.hpp>
#include <ecef_to_geopose_transformation/ecef2geopose_node.hpp>
#include <geographic_msgs/msg/geo_pose_with_covariance_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>
#include <string>

using std::placeholders::_1;

Ecef2GeoposeTransform::Ecef2GeoposeTransform() : rclcpp::Node("ecef_to_geopose_transformer")
{
  input_topic_ = declare_parameter<std::string>("input_topic");
  output_topic_ = declare_parameter<std::string>("output_topic");
  rclcpp::QoS qos_profile(1);                                       // Depth set to 1
  qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);  // Best Effort reliability
  qos_profile.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);       // Volatile durability
  qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);            // Keep last history
  ecef_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
    input_topic_, qos_profile, std::bind(&Ecef2GeoposeTransform::ecef_callback, this, _1));
  geopose_pub_ =
    this->create_publisher<geographic_msgs::msg::GeoPoseWithCovarianceStamped>(output_topic_, 1);
  proc_time_pub_ = this->create_publisher<std_msgs::msg::Float64>("~/proc_time", 1);
}

tf2::Matrix3x3 Ecef2GeoposeTransform::ECEFtoENURotation(double lat_deg, double lon_deg)
{
  // Convert to radians
  double lat = deg2rad(lat_deg);
  double lon = deg2rad(lon_deg);

  double sin_lat = sin(lat);
  double cos_lat = cos(lat);
  double sin_lon = sin(lon);
  double cos_lon = cos(lon);

  tf2::Matrix3x3 R(
    -sin_lon, cos_lon, 0.0, -sin_lat * cos_lon, -sin_lat * sin_lon, cos_lat, cos_lat * cos_lon,
    cos_lat * sin_lon, sin_lat);

  return R;
}

void Ecef2GeoposeTransform::ecef_callback(const nav_msgs::msg::Odometry::SharedPtr odom)
{
  auto proc_start_time = this->now();
  //Computing LLH from ECEF->FP_POI
  double lat, lon, height;
  GeographicLib::Geocentric earth(
    GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f());
  earth.Reverse(
    odom->pose.pose.position.x, odom->pose.pose.position.y, odom->pose.pose.position.z, lat, lon,
    height);  // lat, lon in degrees

  tf2::Matrix3x3 poi_to_ecef_rot(
    tf2::Quaternion(
      odom->pose.pose.orientation.x, odom->pose.pose.orientation.y, odom->pose.pose.orientation.z,
      odom->pose.pose.orientation.w));                 // This is R_POI_ECEF
  auto ecef_to_enu_rot = ECEFtoENURotation(lat, lon);  // This is R_ECEF_ENU
  auto poi_to_enu_rot = ecef_to_enu_rot * poi_to_ecef_rot;

  tf2::Quaternion poi_to_enu_quat;
  poi_to_enu_rot.getRotation(poi_to_enu_quat);

  //Publishing the transformed message
  geographic_msgs::msg::GeoPoseWithCovarianceStamped msg;
  msg.header.stamp =
    odom->header
      .stamp;  //Same stamp since only transform took place. We want this processing with as little delay as possible.

  msg.pose.pose.position.latitude = lat;
  msg.pose.pose.position.longitude = lon;
  msg.pose.pose.position.altitude = height;

  msg.pose.pose.orientation.x = poi_to_enu_quat.x();
  msg.pose.pose.orientation.y = poi_to_enu_quat.y();
  msg.pose.pose.orientation.z = poi_to_enu_quat.z();
  msg.pose.pose.orientation.w = poi_to_enu_quat.w();
  geopose_pub_->publish(msg);
  auto pub_time = this->now();
  std_msgs::msg::Float64 proc_time_msg;
  proc_time_msg.data = (pub_time - proc_start_time).seconds() * 1000.0;
  proc_time_pub_->publish(proc_time_msg);
}
