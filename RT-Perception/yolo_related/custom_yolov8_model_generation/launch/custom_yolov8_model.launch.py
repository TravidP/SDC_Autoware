import os
import yaml

from ament_index_python.packages import get_package_share_directory

import launch
import launch_ros.actions


def generate_launch_description():
    config_common = os.path.join(
    get_package_share_directory('custom_yolov8_model_generation'),
    'config',
    'yolov8_training_config.yaml'
    )

    return launch.LaunchDescription([
        launch_ros.actions.Node(
            package='custom_yolov8_model_generation',
            executable='train',
            name='train',
            parameters=[config_common],
            output='screen'),
  ])