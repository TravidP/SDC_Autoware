cmake_minimum_required(VERSION 3.8)
project(session_recorder)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(rosbag2_storage REQUIRED)

# Include directories
include_directories(include)

# Create executable
add_executable(session_recorder
  src/main.cpp
  src/session_recorder.cpp
)

ament_target_dependencies(session_recorder
  rclcpp
  std_msgs
  std_srvs
  rosbag2_cpp
  rosbag2_storage
)

# Install targets
install(TARGETS
session_recorder
  DESTINATION lib/${PROJECT_NAME}
)

# Install directories
install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
