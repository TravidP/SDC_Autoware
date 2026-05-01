cmake_minimum_required(VERSION 3.8)
project(autonomous_emergency_brake_handler)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(autoware_control_msgs REQUIRED)
find_package(autoware_adapi_v1_msgs REQUIRED)
find_package(autoware_vehicle_msgs REQUIRED)

add_executable(autonomous_emergency_brake_handler src/autonomous_emergency_brake_handler.cpp)
ament_target_dependencies(autonomous_emergency_brake_handler
  rclcpp
  diagnostic_msgs
  autoware_control_msgs
  autoware_adapi_v1_msgs
  autoware_vehicle_msgs
)

install(DIRECTORY
  config
  launch
  DESTINATION share/${PROJECT_NAME}/
)

install(TARGETS
  autonomous_emergency_brake_handler
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
