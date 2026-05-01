cmake_minimum_required(VERSION 3.8)
project(gd_vehicle_interface_rviz_plugin)

# find dependencies
#find_package(ament_cmake REQUIRED)
#find_package(rclcpp REQUIRED)
#find_package(libqt5-core REQUIRED)
#find_package(libqt5-gui REQUIRED)
#find_package(libqt5-widgets REQUIRED)
#find_package(qtbase5-dev REQUIRED)
#find_package(rviz-common REQUIRED)
find_package(autoware_cmake REQUIRED)
autoware_package()


find_package(Qt5 REQUIRED Core Widgets)
set(QT_LIBRARIES Qt5::Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_definitions(-DQT_NO_KEYWORDS)

ament_auto_add_library(${PROJECT_NAME} SHARED
  src/greendino_vehicle_interface_panel.cpp
  src/greendino_vehicle_interface_panel.hpp
)

target_link_libraries(${PROJECT_NAME}
  ${QT_LIBRARIES}
)

# Export the plugin to be imported by rviz2
pluginlib_export_plugin_description_file(rviz_common plugins/plugin_description.xml)

ament_auto_package(
INSTALL_TO_SHARE
plugins
)