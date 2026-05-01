cmake_minimum_required(VERSION 3.8)
project(svo_rosbag_sync)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_msgs REQUIRED)
find_package(autoware_vehicle_msgs REQUIRED)
find_package(autoware_planning_msgs REQUIRED)
find_package(autoware_control_msgs REQUIRED)
find_package(fixposition_driver_ros2 REQUIRED)
find_package(continental_msgs REQUIRED)

add_executable(svo_rosbag_sync src/svo_rosbag_sync.cpp)

ament_target_dependencies(
        svo_rosbag_sync
        rclcpp
        rosbag2_cpp
        sensor_msgs
        geometry_msgs
        nav_msgs
        diagnostic_msgs
        tf2
        tf2_ros
        tf2_msgs
        autoware_vehicle_msgs
        autoware_planning_msgs
        autoware_control_msgs
        fixposition_driver_ros2
        continental_msgs
)

install(TARGETS
        svo_rosbag_sync
        DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
        DESTINATION share/${PROJECT_NAME}
)

ament_package()
