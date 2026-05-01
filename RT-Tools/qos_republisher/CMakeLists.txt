cmake_minimum_required(VERSION 3.8)
project(qos_republisher)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)

add_executable(qos_republisher_node src/qos_republisher_node.cpp)
ament_target_dependencies(qos_republisher_node rclcpp nav_msgs)

install(TARGETS
  qos_republisher_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME}/)

ament_package()
