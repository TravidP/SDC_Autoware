cmake_minimum_required(VERSION 3.8)
project(rt_radar_converter)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(continental_msgs REQUIRED)
find_package(autoware_perception_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

add_executable(rt_radar_converter src/rt_radar_converter.cpp)
ament_target_dependencies(rt_radar_converter
  rclcpp
  visualization_msgs
  continental_msgs
  autoware_perception_msgs
  tf2_ros
  tf2_geometry_msgs
)

install(TARGETS
  rt_radar_converter
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY
  config
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
