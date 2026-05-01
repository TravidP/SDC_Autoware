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
# https://github.com/GreenDinoBV/GD-yolo_ros/blob/gd-main/yolov8_bringup/launch/yolov8.launch.py

import os
import yaml

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration, TextSubstitution
from launch.actions import DeclareLaunchArgument


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
        default_value=os.path.join(
            get_package_share_directory('custom_yolov8_model_generation'),
            'models',
            default_params["model"]
        ),
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

    debug_node_cmd = Node(
        package="yolov8_ros",
        executable="debug_node",
        name="debug_node",
        namespace=namespace,
        parameters=[{"image_reliability": image_reliability}],
        remappings=[
            ("image_raw", input_image_topic),
            ("detections", "tracking")
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
    ld.add_action(namespace_cmd)

    ld.add_action(detector_node_cmd)
    ld.add_action(tracking_node_cmd)
    ld.add_action(debug_node_cmd)

    return ld
