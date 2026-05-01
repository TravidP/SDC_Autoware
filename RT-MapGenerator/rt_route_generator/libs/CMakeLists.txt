# This is the subdirectory CMakeLists.txt for the libs folder

# Map Generator Library
find_package(lanelet2_projection REQUIRED)

add_library(map_generator 
  map_generator/src/map_generator.cpp
)

target_include_directories(map_generator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/map_generator/include>
  $<INSTALL_INTERFACE:include>
  ${Boost_INCLUDE_DIRS}
)

target_link_libraries(map_generator
  ${Boost_GEOMETRY_LIBRARY}
)

ament_target_dependencies(map_generator
  lanelet2_projection
)

# Rosbag Handler Library
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(autoware_map_msgs REQUIRED)
find_package(autoware_lanelet2_extension REQUIRED)
find_package(tf2 REQUIRED)
find_package(fixposition_driver_msgs REQUIRED)
find_package(fixposition_driver_lib REQUIRED)
find_package(fpsdk_common REQUIRED)

add_library(rosbag_handler 
  rosbag_handler/src/rosbag_handler.cpp
)

target_include_directories(rosbag_handler PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rosbag_handler/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(rosbag_handler
  sensor_msgs
  nav_msgs
  rosbag2_cpp
  autoware_map_msgs
  autoware_lanelet2_extension
  tf2
  fixposition_driver_msgs
  fixposition_driver_lib
  fpsdk_common
)

# Route Creator Library
find_package(nav_msgs REQUIRED)
find_package(autoware_lanelet2_extension REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(fixposition_driver_msgs REQUIRED)
find_package(fixposition_driver_lib REQUIRED)
find_package(fpsdk_common REQUIRED)

add_library(route_creator 
  route_creator/src/route_creator.cpp
)

target_include_directories(route_creator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/route_creator/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(route_creator
  nav_msgs
  autoware_lanelet2_extension
  builtin_interfaces
  fixposition_driver_msgs
  fixposition_driver_lib
  fpsdk_common
)

# Rosbag Recorder Library
find_package(ament_cmake REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem system)

add_library(rosbag_recorder rosbag_recorder/src/rosbag_recorder.cpp)

target_include_directories(
  rosbag_recorder PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rosbag_recorder/include>
                         ${Boost_INCLUDE_DIRS}>)

target_link_libraries(rosbag_recorder
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

install(DIRECTORY rosbag_recorder/include DESTINATION include)
