cmake_minimum_required(VERSION 3.8)
project(diagnostics_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(autoware_vehicle_msgs REQUIRED)


add_executable(diagnostics_controller src/main.cpp src/diagnostics_controller.cpp)

target_include_directories(diagnostics_controller
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(diagnostics_controller
  rclcpp
  diagnostic_msgs
  autoware_vehicle_msgs
)

install(TARGETS
  diagnostics_controller
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}/)

ament_package()
