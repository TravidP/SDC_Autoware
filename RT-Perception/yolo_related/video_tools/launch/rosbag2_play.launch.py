import os
import yaml

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration, PythonExpression


def generate_launch_description():

    # Define LaunchConfiguration objects for rosbag2_path and rosbag2_topics
    rosbag2_path = LaunchConfiguration('rosbag2_path')
    rosbag2_topics = LaunchConfiguration('rosbag2_topics')

    # Declare launch arguments for rosbag2_path and rosbag2_topics
    rosbag2_path_args = DeclareLaunchArgument(
        'rosbag2_path',
        default_value='/home/greendinokubra/rosbag2_bus/rosbag2_2024_01_26-16_26_22_0.db3',
        description='The directory path where the rosbag2 file is located'
    )

    rosbag2_topics_args = DeclareLaunchArgument(
        'rosbag2_topics',
        default_value='/zed/zed_node/rgb_raw/camera_info /zed/zed_node/rgb_raw/image_raw_color /zed/zed_node/depth/camera_info /zed/zed_node/confidence/confidence_map /tf /tf_static /zed/zed_node/depth/depth_registered'
    )

    return LaunchDescription(
        [
            rosbag2_path_args, 
            rosbag2_topics_args, 
            # ExecuteProcess block to play the rosbag2 file with all topics
            ExecuteProcess
            (
                condition=IfCondition(PythonExpression(["'", rosbag2_topics, "' == '' or ", "'", rosbag2_topics, "' == 'all'"])),
                cmd=[['ros2 bag play ', rosbag2_path]],
                shell=True,
            ),
            # ExecuteProcess block to play the rosbag2 file with specific topics
            ExecuteProcess
            (
                condition=UnlessCondition(PythonExpression(["'", rosbag2_topics, "' == ''"])),
                cmd=[['ros2 bag play ', rosbag2_path, ' --topics ', rosbag2_topics]],
                shell=True,
            )
        ]
    )