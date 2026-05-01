import os
import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def load_yaml(yaml_file):
    with open(yaml_file, 'r') as file:
        return yaml.safe_load(file)

def generate_launch_description():
    config_file = os.path.join(get_package_share_directory('gd_citaro_sensor_description'), 'config', 'sensors_calibration.yaml')
    config = load_yaml(config_file)

    frame_transform_x = '-' + str(config['base_link']['gnss_link']['x'])
    frame_transform_y = '-' + str(config['base_link']['gnss_link']['y'])
    frame_transform_z = '-' + str(config['base_link']['gnss_link']['z'])
    # These need to be properly loaded
    frame_transform_roll = '0'
    frame_transform_pitch = '0'
    frame_transform_yaw = '0'

    # Create a static_transform_publisher Node
    static_transform_publisher_1 = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher_1',
        output='screen',
        arguments=[
            frame_transform_x , frame_transform_y, frame_transform_z, frame_transform_roll, frame_transform_pitch, frame_transform_yaw,
            'gnss_fp_poi',
            'base_link'
        ]
    )
    return LaunchDescription([
        static_transform_publisher_1
    ])