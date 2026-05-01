cmake_minimum_required(VERSION 3.8)
project(object_detection_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(zed_interfaces REQUIRED)
find_package(autoware_perception_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(yolov8_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

include_directories(
  include
)

add_executable(object_detection_zed_bridge src/object_detection_zed_bridge.cpp)

ament_target_dependencies(object_detection_zed_bridge
  rclcpp
  sensor_msgs
  geometry_msgs
  zed_interfaces
  autoware_perception_msgs
  tf2_ros
  tf2_geometry_msgs
)

add_executable(object_detection_yolo_bridge src/object_detection_yolo_bridge.cpp)

ament_target_dependencies(object_detection_yolo_bridge
  rclcpp
  sensor_msgs
  geometry_msgs
  autoware_perception_msgs
  tf2_ros
  tf2_geometry_msgs
  yolov8_msgs
  nav_msgs
)

install(TARGETS
  object_detection_zed_bridge
  object_detection_yolo_bridge
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
