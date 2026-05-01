cmake_minimum_required(VERSION 3.8)
project(rt_vehicle_interface)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

ament_auto_add_executable(rt_vehicle_interface
  src/main.cpp
  src/ros_vehicle_adapter.hpp
  src/rt_vehicle_interface.hpp
)

ament_auto_package(
  INSTALL_TO_SHARE
  launch
  config
)
