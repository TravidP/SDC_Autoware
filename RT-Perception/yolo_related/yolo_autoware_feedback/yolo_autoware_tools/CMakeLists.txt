cmake_minimum_required(VERSION 3.8)
project(yolo_autoware_tools)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(PugiXML REQUIRED)
find_package(lanelet2_io REQUIRED)
find_package(autoware_lanelet2_extension REQUIRED)
find_package(yaml-cpp REQUIRED)

find_library(PUGIXML_LIBRARIES
  NAMES pugixml
)

find_path(PUGIXML_INCLUDE_DIRS
  NAMES pugixml.hpp
  PATH_SUFFIXES pugixml
)

include_directories(
  ${PUGIXML_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIRS}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

include_directories(
  include
)

add_executable(nearest_traffic_light src/nearest_traffic_light.cpp)

ament_target_dependencies(nearest_traffic_light
  rclcpp
  std_msgs
  sensor_msgs
  nav_msgs
  lanelet2_io
  autoware_lanelet2_extension
  yaml-cpp
)
# Link PugiXML library
target_link_libraries(nearest_traffic_light 
  ${PUGIXML_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(TARGETS
  nearest_traffic_light
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
