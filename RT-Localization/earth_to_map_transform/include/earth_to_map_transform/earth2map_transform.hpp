#ifndef EARTH2MAP_TRANSFORM_HPP
#define EARTH2MAP_TRANSFORM_HPP

#include <tf2_ros/buffer.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>

#include <autoware_map_msgs/msg/map_projector_info.hpp>
#include <geographic_msgs/msg/geo_point.hpp>
#include <memory>
#include <rclcpp/rclcpp.hpp>

class Earth2MapTransform : public rclcpp::Node
{
public:
  Earth2MapTransform();

private:
  double lookup_timeout_;
  geographic_msgs::msg::GeoPoint map_proj_info_;
  geometry_msgs::msg::TransformStamped ecef2enu0_transform_;
  geometry_msgs::msg::TransformStamped ecefmap_transform_;
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  std::shared_ptr<tf2_ros::StaticTransformBroadcaster> broadcaster_;

  void map_proj_info_callback(const autoware_map_msgs::msg::MapProjectorInfo::SharedPtr msg);
  void wgs84ToEcef(
    double latitude, double longitude, double altitude, double & x, double & y, double & z);

  rclcpp::Subscription<autoware_map_msgs::msg::MapProjectorInfo>::SharedPtr map_proj_info_subs_;
};

#endif  // EARTH2MAP_TRANSFORM_HPP