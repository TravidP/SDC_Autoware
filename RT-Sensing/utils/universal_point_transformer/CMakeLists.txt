cmake_minimum_required(VERSION 3.8)
project(universal_point_transformer)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(autoware_point_types REQUIRED)

# Add universal point transformer component library
add_library(universal_point_transformer_component SHARED
  src/universal_point_transformer.cpp
)
target_include_directories(universal_point_transformer_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  # ${autoware_point_types_INCLUDE_DIRS
  }
)
target_compile_features(universal_point_transformer_component PUBLIC c_std_99 cxx_std_17)
ament_target_dependencies(universal_point_transformer_component
  rclcpp
  rclcpp_components
  sensor_msgs
  autoware_point_types
)

# Register the component
rclcpp_components_register_node(universal_point_transformer_component
  PLUGIN "universal_point_transformer::UniversalPointTransformerComponent"
  EXECUTABLE universal_point_transformer_node
)

# Install
install(
  TARGETS universal_point_transformer_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)

# Install launch files
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
