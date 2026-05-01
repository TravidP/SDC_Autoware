cmake_minimum_required(VERSION 3.8)
project(earth_to_map_transform)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geographic_msgs REQUIRED)
find_package(tier4_map_msgs REQUIRED)
find_package(autoware_map_msgs REQUIRED)
find_package(geographic_msgs REQUIRED)

# GeographicLib
find_package(PkgConfig)
find_path(GeographicLib_INCLUDE_DIR GeographicLib/Config.h
  PATH_SUFFIXES GeographicLib
)
set(GeographicLib_INCLUDE_DIRS ${GeographicLib_INCLUDE_DIR})
find_library(GeographicLib_LIBRARIES NAMES Geographic)

include_directories(
  include  # This includes the header files in your package's include directory.
  )

add_executable(earth2map_transform
  src/earth2map_transform.cpp
)
ament_target_dependencies(earth2map_transform 
  rclcpp 
  geometry_msgs 
  tf2_ros
  geographic_msgs
  autoware_map_msgs
  geographic_msgs
)

target_link_libraries(earth2map_transform
  Geographic
)

install(TARGETS
  earth2map_transform
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

ament_package()
