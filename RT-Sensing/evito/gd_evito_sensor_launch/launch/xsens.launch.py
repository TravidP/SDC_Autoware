import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Default path for configuration file
    default_config_path = os.path.join(get_package_share_directory('gd_evito_sensor_launch'), 'config', 'xsens', 'xsens.yaml')

    return LaunchDescription([
        DeclareLaunchArgument(
            'config_path',
            default_value=default_config_path,
            description='Path to the YAML configuration file for the camera.'
        ),
        Node(
            package='fixposition_driver_ros2',
            # namespace='fixposition_driver_ros2',
            executable='fixposition_driver_ros2_exec',
            # name='fixposition_driver_ros2',
            parameters=[LaunchConfiguration('config_path')],
        ),
    ])
