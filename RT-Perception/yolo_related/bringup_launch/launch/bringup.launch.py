import os

import yaml
from ament_index_python import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch_ros.substitutions import FindPackageShare
from launch.launch_description_sources import AnyLaunchDescriptionSource, PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution, LaunchConfiguration

def generate_launch_description():

    # Load default bringup configuration from YAML file
    default_bringup_config = os.path.join(
        get_package_share_directory('bringup_launch'),
        'config',
        'bringup_config.yaml'
    )

    with open(default_bringup_config, 'r') as file:
            bringup_config = yaml.safe_load(file)

    # Extract configuration details for object_detection_launch
    object_detection_launch = bringup_config["object_detection_launch"]

    # Path to the custom params file for yolo_feedback
    yolo_feedback_params_file = os.path.join(
        get_package_share_directory('bringup_launch'),
        'config',
        'yolo_feedback_params.yaml'
    )

    # Path to the point cloud and lanelet2 map directory
    map_path = LaunchConfiguration('map_path')
    
    return LaunchDescription([
        # Declare launch argument for map_path
        DeclareLaunchArgument(
            'map_path',
            default_value='',
            description='Point cloud and lanelet2 map directory path'
        ),
        # Declare launch argument for params_file
        DeclareLaunchArgument(
            'params_file',
            default_value=yolo_feedback_params_file,
            description='Full path to the custom parameters file for yolo_feedback'
        ),
        # Include launch file for object detection
        IncludeLaunchDescription(
            AnyLaunchDescriptionSource([
                FindPackageShare("object_detection_launch"), '/launch', '/detection.launch.xml']),
                launch_arguments={
                   'enable_3d_object_detection': object_detection_launch["enable_3d_object_detection"],
                   'rviz': object_detection_launch["rviz"],
                   'tracker': object_detection_launch["tracker"],
                   'model': PathJoinSubstitution([FindPackageShare('custom_yolov8_model_generation'), 'models', object_detection_launch["model"]]), #object_detection_launch["model"],
                   'device': object_detection_launch["device"],
                   'enable': object_detection_launch["enable"],
                   'threshold': object_detection_launch["threshold"],
                   'image_reliability': object_detection_launch["image_reliability"],
                   'depth_image_reliability': object_detection_launch["depth_image_reliability"],
                   'depth_info_reliability': object_detection_launch["depth_info_reliability"],
                   'depth_image_units_divisor': object_detection_launch["depth_image_units_divisor"],
                   'maximum_detection_threshold': object_detection_launch["maximum_detection_threshold"],
                   'input_image_topic': object_detection_launch["input_image_topic"],
                   'input_depth_info_topic': object_detection_launch["input_depth_info_topic"],
                   'input_depth_topic': object_detection_launch["input_depth_topic"],
                   'target_frame': object_detection_launch["target_frame"],
                }.items(),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([
                FindPackageShare("yolo_autoware_feedback"), '/launch', '/yolo_feedback.launch.py']),
                launch_arguments={
                    'params_file': LaunchConfiguration('params_file')
                }.items(),
        ),
        IncludeLaunchDescription(
            AnyLaunchDescriptionSource([
                FindPackageShare("yolo_autoware_tools"), '/launch', '/nearest_traffic_light.launch.xml']),
                launch_arguments={'map_path': map_path}.items()
        ),
    ])

