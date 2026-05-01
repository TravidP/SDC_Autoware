from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
import os
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    # Default path to the YAML file
    default_params_file = os.path.join(
        get_package_share_directory('yolo_autoware_feedback'),
        'config',
        'params.yaml'
    )
    
    return LaunchDescription([
        DeclareLaunchArgument(
            'params_file',
            default_value=default_params_file,
            description='Full path to the parameters file'
        ),
        Node(
            package='yolo_autoware_feedback',
            executable='yolo_feedback',
            name='yolo_feedback',
            output='log',
            parameters=[LaunchConfiguration('params_file')]
        )
    ])
