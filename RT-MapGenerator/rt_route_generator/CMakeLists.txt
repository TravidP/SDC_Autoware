cmake_minimum_required(VERSION 3.8)
project(rt_route_generator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(autoware_vehicle_info_utils)
find_package(rt_route_generator_msgs REQUIRED)

# add_subdirectory(rosbag_recorder)
add_subdirectory(libs)

add_executable(route_generator src/main.cpp src/route_generator_node.cpp)

target_include_directories(route_generator PUBLIC include)
ament_target_dependencies(route_generator rclcpp rt_route_generator_msgs autoware_vehicle_info_utils)

target_link_libraries(route_generator rosbag_recorder map_generator
                      rosbag_handler route_creator)

install(TARGETS route_generator DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME})

ament_package()
