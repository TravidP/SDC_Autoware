# Copyright (C) 2023  Miguel Ángel González Santamarta

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


###################
# This is a copy-paste from the following link with added YAML file loading:
# https://github.com/GreenDinoBV/GD-yolo_ros/blob/gd-main/yolov8_bringup/launch/yolov8_3d.launch.py

import os
import yaml

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():

    default_config_common = os.path.join(
    get_package_share_directory('object_detection_launch'),
    'config',
    'yolo_config.yaml'
    )

    with open(default_config_common, 'r') as file:
            config_data = yaml.safe_load(file)


    default_params = config_data["yolo"]["default_params"]
    remapped_topics = config_data["yolo"]["remapped_topics"]

    #
    # ARGS
    #
    model = LaunchConfiguration("model")
    model_cmd = DeclareLaunchArgument(
        "model",
        default_value="yolov8n.pt",
        description="Model name or path")

    tracker = LaunchConfiguration("tracker")
    tracker_cmd = DeclareLaunchArgument(
        "tracker",
        default_value=default_params["tracker"],
        description="Tracker name or path")

    device = LaunchConfiguration("device")
    device_cmd = DeclareLaunchArgument(
        "device",
        default_value=default_params["device"],
        description="Device to use (GPU/CPU)")

    enable = LaunchConfiguration("enable")
    enable_cmd = DeclareLaunchArgument(
        "enable",
        default_value=default_params["enable"],
        description="Whether to start YOLOv8 enabled")

    threshold = LaunchConfiguration("threshold")
    threshold_cmd = DeclareLaunchArgument(
        "threshold",
        default_value=default_params["threshold"],
        description="Minimum probability of a detection to be published")

    input_image_topic = LaunchConfiguration("input_image_topic")
    input_image_topic_cmd = DeclareLaunchArgument(
        "input_image_topic",
        default_value=remapped_topics["input_image_topic"],
        description="Name of the input image topic")

    image_reliability = LaunchConfiguration("image_reliability")
    image_reliability_cmd = DeclareLaunchArgument(
        "image_reliability",
        default_value=default_params["image_reliability"],
        choices=["0", "1", "2"],
        description="Specific reliability QoS of the input image topic (0=system default, 1=Reliable, 2=Best Effort)")

    input_depth_topic = LaunchConfiguration("input_depth_topic")
    input_depth_topic_cmd = DeclareLaunchArgument(
        "input_depth_topic",
        default_value=remapped_topics["input_depth_topic"],
        description="Name of the input depth topic")

    depth_image_reliability = LaunchConfiguration("depth_image_reliability")
    depth_image_reliability_cmd = DeclareLaunchArgument(
        "depth_image_reliability",
        default_value=default_params["depth_image_reliability"],
        choices=["0", "1", "2"],
        description="Specific reliability QoS of the input depth image topic (0=system default, 1=Reliable, 2=Best Effort)")

    input_depth_info_topic = LaunchConfiguration("input_depth_info_topic")
    input_depth_info_topic_cmd = DeclareLaunchArgument(
        "input_depth_info_topic",
        default_value=remapped_topics["input_depth_info_topic"],
        description="Name of the input depth info topic")

    depth_info_reliability = LaunchConfiguration("depth_info_reliability")
    depth_info_reliability_cmd = DeclareLaunchArgument(
        "depth_info_reliability",
        default_value=default_params["depth_info_reliability"],
        choices=["0", "1", "2"],
        description="Specific reliability QoS of the input depth info topic (0=system default, 1=Reliable, 2=Best Effort)")

    depth_image_units_divisor = LaunchConfiguration(
        "depth_image_units_divisor")
    depth_image_units_divisor_cmd = DeclareLaunchArgument(
        "depth_image_units_divisor",
        default_value=default_params["depth_image_units_divisor"],
        description="Divisor used to convert the raw depth image values into metres")

    target_frame = LaunchConfiguration("target_frame")
    target_frame_cmd = DeclareLaunchArgument(
        "target_frame",
        default_value=remapped_topics["target_frame"],
        description="Target frame to transform the 3D boxes")

    maximum_detection_threshold = LaunchConfiguration(
        "maximum_detection_threshold")
    maximum_detection_threshold_cmd = DeclareLaunchArgument(
        "maximum_detection_threshold",
        default_value=default_params["maximum_detection_threshold"],
        description="Maximum detection threshold in the z axis")

    namespace = LaunchConfiguration("namespace")
    namespace_cmd = DeclareLaunchArgument(
        "namespace",
        default_value="yolo",
        description="Namespace for the nodes")

    #
    # NODES
    #
    detector_node_cmd = Node(
        package="yolov8_ros",
        executable="yolov8_node",
        name="yolov8_node",
        namespace=namespace,
        parameters=[{
            "model": model,
            "device": device,
            "enable": enable,
            "threshold": threshold,
            "image_reliability": image_reliability,
        }],
        remappings=[("image_raw", input_image_topic)]
    )

    tracking_node_cmd = Node(
        package="yolov8_ros",
        executable="tracking_node",
        name="tracking_node",
        namespace=namespace,
        parameters=[{
            "tracker": tracker,
            "image_reliability": image_reliability
        }],
        remappings=[("image_raw", input_image_topic)]
    )

    detect_3d_node_cmd = Node(
        package="yolov8_ros",
        executable="detect_3d_node",
        name="detect_3d_node",
        namespace=namespace,
        parameters=[{
            "target_frame": target_frame,
            "maximum_detection_threshold": maximum_detection_threshold,
            "depth_image_units_divisor": depth_image_units_divisor,
            "depth_image_reliability": depth_image_reliability,
            "depth_info_reliability": depth_info_reliability
        }],
        remappings=[
            ("depth_image", input_depth_topic),
            ("depth_info", input_depth_info_topic),
            ("detections", "tracking")
        ]
    )

    debug_node_cmd = Node(
        package="yolov8_ros",
        executable="debug_node",
        name="debug_node",
        namespace=namespace,
        parameters=[{"image_reliability": image_reliability}],
        remappings=[
            ("image_raw", input_image_topic),
            ("detections", "detections_3d")
        ]
    )

    ld = LaunchDescription()

    ld.add_action(model_cmd)
    ld.add_action(tracker_cmd)
    ld.add_action(device_cmd)
    ld.add_action(enable_cmd)
    ld.add_action(threshold_cmd)
    ld.add_action(input_image_topic_cmd)
    ld.add_action(image_reliability_cmd)
    ld.add_action(input_depth_topic_cmd)
    ld.add_action(depth_image_reliability_cmd)
    ld.add_action(input_depth_info_topic_cmd)
    ld.add_action(depth_info_reliability_cmd)
    ld.add_action(depth_image_units_divisor_cmd)
    ld.add_action(target_frame_cmd)
    ld.add_action(maximum_detection_threshold_cmd)
    ld.add_action(namespace_cmd)

    ld.add_action(detector_node_cmd)
    ld.add_action(tracking_node_cmd)
    ld.add_action(detect_3d_node_cmd)
    ld.add_action(debug_node_cmd)

    return ld
