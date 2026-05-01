cmake_minimum_required(VERSION 3.8)
project(automatic_goal_publisher)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(autoware_adapi_v1_msgs REQUIRED)
find_package(autoware_planning_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(tier4_planning_msgs REQUIRED)
find_package(autoware_internal_msgs REQUIRED)

# Include directories
include_directories(
  include 
  ${YAML_CPP_INCLUDE_DIRS}
)

add_executable(publish_goal
  src/publish_goal.cpp
)
ament_target_dependencies(publish_goal 
  rclcpp 
  geometry_msgs
  nav_msgs
  autoware_adapi_v1_msgs
  autoware_planning_msgs
  tier4_planning_msgs
  autoware_internal_msgs
)

target_link_libraries(publish_goal
  ${YAML_CPP_LIBRARIES}
)

install(TARGETS
  publish_goal
  DESTINATION lib/${PROJECT_NAME}
)

# Install directory for header files
install(
  DIRECTORY include/
  DESTINATION include
)

# Install launch files
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

# Install config files
install(DIRECTORY
  config/
  DESTINATION config
)



ament_package()
