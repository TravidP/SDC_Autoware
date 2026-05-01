
#include "rosbag_handler/rosbag_handler.hpp"
#include "rosbag_handler/gd_conversion.hpp"

RosbagHandler::Result RosbagHandler::open_rosbag(const std::string & bag_filename)
{
  Result result;

  try {
    rosbag_reader.open(bag_filename);
    double previous_map_x = 0.0;
    double previous_map_y = 0.0;

    while (rosbag_reader.has_next()) {
      rosbag2_storage::SerializedBagMessageSharedPtr msg = rosbag_reader.read_next();

      if (msg->topic_name == "/fixposition/fpa/llh") {
        rclcpp::SerializedMessage serialized_msg(*msg->serialized_data);
        fixposition_driver_msgs::msg::FpaLlh navsatfix;

        serializer.deserialize_message(&serialized_msg, &navsatfix);

        double map_x = navsatfix.position.x;
        double map_y = navsatfix.position.y;
        double distance_to_previous =
          haversine_distance(previous_map_x, previous_map_y, map_x, map_y);

        // 30cm distance between points
        if (distance_to_previous > 0.3) {
          result.navsatfix_coordinates.push_back(
            std::make_pair(navsatfix.position.x, navsatfix.position.y));
          result.fixposition_navsatfix_message.push_back(navsatfix);

          previous_map_x = map_x;
          previous_map_y = map_y;
        }

      } else if (msg->topic_name == "/ecef_to_base_link/odometry") {
        rclcpp::SerializedMessage serialized_msg_ecef(*msg->serialized_data);
        nav_msgs::msg::Odometry ecef_to_baselink_odom;
        serializer_ecef_to_baselink_odom.deserialize_message(
          &serialized_msg_ecef, &ecef_to_baselink_odom);

        result.ecef_to_baselink_message.push_back(ecef_to_baselink_odom);

      } else if (msg->topic_name == "/fixposition/fpa/odometry") {
        rclcpp::SerializedMessage serialized_msg_odom(*msg->serialized_data);
        fixposition_driver_msgs::msg::FpaOdometry fixposition_odom;
        serializer_fix_position_odom.deserialize_message(&serialized_msg_odom, &fixposition_odom);

        result.fixposition_odometry_message.push_back(fixposition_odom);

      } else if (msg->topic_name == "/localization/kinematic_state") {
        rclcpp::SerializedMessage serialized_msg_kinematic(*msg->serialized_data);
        nav_msgs::msg::Odometry kinematic_state;
        serializer_loc.deserialize_message(&serialized_msg_kinematic, &kinematic_state);

        result.kinematic_state_message.push_back(kinematic_state);

      } else if (msg->topic_name == "/map/map_projector_info") {
        rclcpp::SerializedMessage serialized_msg_map_info(*msg->serialized_data);
        autoware_map_msgs::msg::MapProjectorInfo map_projector_info;
        serializer_map.deserialize_message(&serialized_msg_map_info, &map_projector_info);

        result.map_projector_message = map_projector_info;

      } else {
        continue;
      }
    }

    rosbag_reader.close();

  } catch (const std::exception & e) {
    std::cerr << "Error opening ROS bag file: " << e.what() << std::endl;
  }
  return result;
}
