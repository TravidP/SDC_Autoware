cmake_minimum_required(VERSION 3.8)
project(yolo_autoware_msgs)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(yolov8_msgs REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/CustomImage.msg"
  "msg/DetectionHeader.msg"
  DEPENDENCIES std_msgs sensor_msgs yolov8_msgs
)

ament_package()
