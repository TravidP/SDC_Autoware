cmake_minimum_required(VERSION 3.8)
project(xsens_transform)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(fixposition_driver_msgs REQUIRED)
find_package(fixposition_driver_lib REQUIRED)
find_package(fpsdk_common REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geodesy REQUIRED)
find_package(geographic_msgs REQUIRED)

add_executable(xsens_transform src/xsens_transform.cpp)
ament_target_dependencies(xsens_transform 
  rclcpp 
  geometry_msgs
  tf2
  tf2_ros
  tf2_msgs
  tf2_geometry_msgs
  geodesy
  fixposition_driver_msgs
  fixposition_driver_lib
  fpsdk_common
  geographic_msgs
  nav_msgs
)

target_link_libraries(xsens_transform
  Geographic
)

install(TARGETS
  xsens_transform
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
