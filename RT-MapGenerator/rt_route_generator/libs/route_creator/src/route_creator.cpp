
#include "route_creator/route_creator.hpp"

#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Transform.h>
#include <tf2/LinearMath/Vector3.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "autoware_lanelet2_extension/projection/transverse_mercator_projector.hpp"

double RouteCreator::convert_time_to_seconds(
  const builtin_interfaces::msg::Time & time_msg) const noexcept
{
  double nanoseconds_in_seconds = static_cast<double>(time_msg.nanosec) * 1e-9;
  return static_cast<double>(time_msg.sec) + nanoseconds_in_seconds;
}

void RouteCreator::generate_route_from_navsatfix(
  const std::vector<fixposition_driver_msgs::msg::FpaLlh> & fixposition_navsatfix_message,
  const std::string & output_folder)
{
  auto projectPoint =
    [&fixposition_navsatfix_message](const geometry_msgs::msg::Vector3 & gps_point) {
      auto projector = lanelet::projection::TransverseMercatorProjector(lanelet::Origin(
        {fixposition_navsatfix_message.front().position.x,
         fixposition_navsatfix_message.front().position.y}));
      lanelet::GPSPoint point;
      point.lat = gps_point.x;
      point.lon = gps_point.y;
      lanelet::BasicPoint3d point_3d = projector.forward(point);
      return tf2::Vector3(point_3d.x(), point_3d.y(), 0.);
    };

  std::ofstream route_filestream(output_folder + "/route_navsatfix.csv");
  route_filestream << std::fixed << std::setprecision(9);
  route_filestream << "pose_x,pose_y,pose_z,orientation_x,orientation_y,"
                      "orientation_z,orientation_w,timestamp\n";

  for (auto it = fixposition_navsatfix_message.cbegin();
       std::next(it) < fixposition_navsatfix_message.cend(); it++) {
    tf2::Transform projected_pose(tf2::Quaternion(), projectPoint(it->position));
    tf2::Transform projected_pose_next(tf2::Quaternion(), projectPoint(std::next(it)->position));

    double yaw = std::atan2(
      (projected_pose_next.getOrigin() - projected_pose.getOrigin()).y(),
      (projected_pose_next.getOrigin() - projected_pose.getOrigin()).x());
    tf2::Quaternion quaternion;
    quaternion.setRPY(0, 0, yaw);
    projected_pose.setRotation(quaternion);
    write_stamped_transform_to_file(
      projected_pose, convert_time_to_seconds(it->header.stamp), route_filestream);
  }
  route_filestream.close();
}

void RouteCreator::generate_route_ecef_to_base_link(
  const std::vector<nav_msgs::msg::Odometry> & ecef_to_baselink_message,
  const std::string & output_folder) const
{
  std::ofstream route_filestream(output_folder + "/vehicle_pose.csv");
  route_filestream << std::fixed << std::setprecision(6);
  route_filestream << "pose_x,pose_y,pose_z,orientation_x,orientation_y,"
                      "orientation_z,orientation_w,timestamp\n";
  std::for_each(
    ecef_to_baselink_message.cbegin(), ecef_to_baselink_message.cend(),
    [this, &route_filestream](const nav_msgs::msg::Odometry & msg) {
      write_odom_message_to_file(msg, route_filestream);
    });
  route_filestream.close();
}

void RouteCreator::write_odom_message_to_file(
  const nav_msgs::msg::Odometry & msg, std::ofstream & filestream) const noexcept
{
  if (filestream.good()) {
    filestream << msg.pose.pose.position.x << "," << msg.pose.pose.position.y << ","
               << msg.pose.pose.position.z << "," << msg.pose.pose.orientation.x << ","
               << msg.pose.pose.orientation.y << "," << msg.pose.pose.orientation.z << ","
               << msg.pose.pose.orientation.w << "," << convert_time_to_seconds(msg.header.stamp)
               << "\n";
  } else {
    std::cerr << "filestream in error "
                 "state!(RouteCreator::write_odom_message_to_file)\n";
  }
  return;
}

void RouteCreator::write_stamped_transform_to_file(
  const tf2::Transform & transform, double timestamp, std::ofstream & filestream) const noexcept
{
  if (filestream.good()) {
    filestream << transform.getOrigin().x() << "," << transform.getOrigin().y() << ","
               << transform.getOrigin().z() << "," << transform.getRotation().x() << ","
               << transform.getRotation().y() << "," << transform.getRotation().z() << ","
               << transform.getRotation().w() << "," << timestamp << "\n";
  } else {
    std::cerr << "filestream in error "
                 "state!(RouteCreator::write_stamped_transform_to_file)\n";
  }
  return;
}
