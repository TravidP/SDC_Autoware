import os
import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def load_yaml(yaml_file):
    with open(yaml_file, 'r') as file:
        return yaml.safe_load(file)

def generate_launch_description():
    config_file = os.path.join(get_package_share_directory('gd_twizy_sensor_description'), 'config', 'sensors_calibration.yaml')
    config = load_yaml(config_file)

    frame_transform_x = float(config['base_link']['laserscanner_right']['x'])
    frame_transform_y = float(config['base_link']['laserscanner_right']['y'])
    frame_transform_z = float(config['base_link']['laserscanner_right']['z'])
    # These need to be properly loaded
    frame_transform_roll = float(config['base_link']['laserscanner_right']['roll'])
    frame_transform_pitch = float(config['base_link']['laserscanner_right']['pitch'])
    frame_transform_yaw = float(config['base_link']['laserscanner_right']['yaw'])

    # Create a static_transform_publisher Node
    static_transform_publisher_ls_right = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher_ls_right',
        output='screen',
        arguments=[
            str(frame_transform_x) , str(frame_transform_y), str(frame_transform_z), str(frame_transform_roll), str(frame_transform_pitch), str(frame_transform_yaw),
            'base_link',
            'laser_right'
        ]
    )
    return LaunchDescription([
        static_transform_publisher_ls_right
    ])
