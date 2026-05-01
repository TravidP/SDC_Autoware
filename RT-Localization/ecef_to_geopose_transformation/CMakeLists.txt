cmake_minimum_required(VERSION 3.8)
project(ecef_to_geopose_transformation)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geographic_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2 REQUIRED)

# GeographicLib
find_package(PkgConfig)
find_path(GeographicLib_INCLUDE_DIR GeographicLib/Config.h
  PATH_SUFFIXES GeographicLib
)
set(GeographicLib_INCLUDE_DIRS ${GeographicLib_INCLUDE_DIR})
find_library(GeographicLib_LIBRARIES NAMES Geographic)

# Create executable
add_executable(ecef2geopose src/main.cpp src/ecef2geopose_node.cpp)

# Include directories (preferred modern CMake)
target_include_directories(ecef2geopose
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies to executable
ament_target_dependencies(ecef2geopose
  rclcpp
  std_msgs
  geographic_msgs
  nav_msgs
  tf2
)


target_link_libraries(ecef2geopose
  ${GeographicLib_LIBRARIES}
)

install(TARGETS
ecef2geopose
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}/)

ament_package()
