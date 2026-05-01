# Copyright 2024 Stereolabs
#
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    OpaqueFunction,
    LogInfo
)
from launch.conditions import IfCondition
from launch.substitutions import (
    LaunchConfiguration,
    Command,
    TextSubstitution
)
from launch_ros.actions import Node

# Enable colored output
os.environ["RCUTILS_COLORIZED_OUTPUT"] = "1"

# URDF/xacro file to be loaded by the Robot State Publisher node
default_xacro_path = os.path.join(
    get_package_share_directory('zed_wrapper'),
    'urdf',
    'zed_descr.urdf.xacro'
)


def parse_array_param(param):
    str = param.replace('[', '')
    str = str.replace(']', '')
    arr = str.split(',')

    return arr


def launch_setup(context, *args, **kwargs):
    return_array = []

    # Launch configuration variables
    namespace = LaunchConfiguration('namespace')
    camera_name = LaunchConfiguration('camera_name')
    camera_model = LaunchConfiguration('camera_model')
    publish_urdf = LaunchConfiguration('publish_urdf')
    xacro_path = LaunchConfiguration('xacro_path')
    custom_baseline = LaunchConfiguration('custom_baseline')
    enable_gnss = LaunchConfiguration('enable_gnss')
    gnss_antenna_offset = LaunchConfiguration('gnss_antenna_offset')

    namespace_val = namespace.perform(context)
    camera_name_val = camera_name.perform(context)
    camera_model_val = camera_model.perform(context)
    enable_gnss_val = enable_gnss.perform(context)
    gnss_coords = parse_array_param(gnss_antenna_offset.perform(context))
    custom_baseline_val = custom_baseline.perform(context)

    if (camera_name_val == ''):
        camera_name_val = 'zed'

    if (camera_model_val == 'virtual' and float(custom_baseline_val) <= 0):
        return [
            LogInfo(msg="Please set a positive value for the 'custom_baseline' argument when using a 'virtual' Stereo Camera with two ZED X One devices."),
        ]
    
    if(namespace_val == ''):
        namespace_val = camera_name_val

    # Xacro command with options
    xacro_command = []
    xacro_command.append('xacro')
    xacro_command.append(' ')
    xacro_command.append(xacro_path.perform(context))
    xacro_command.append(' ')
    xacro_command.append('camera_name:=')
    xacro_command.append(camera_name_val)
    xacro_command.append(' ')
    xacro_command.append('camera_model:=')
    xacro_command.append(camera_model_val)
    xacro_command.append(' ')
    xacro_command.append('custom_baseline:=')
    xacro_command.append(custom_baseline_val)   
    if(enable_gnss_val=='true'):
        xacro_command.append(' ')
        xacro_command.append('enable_gnss:=true')
        xacro_command.append(' ')
        if(len(gnss_coords)==3):
            xacro_command.append('gnss_x:=')
            xacro_command.append(gnss_coords[0])
            xacro_command.append(' ')
            xacro_command.append('gnss_y:=')
            xacro_command.append(gnss_coords[1])
            xacro_command.append(' ')
            xacro_command.append('gnss_z:=')
            xacro_command.append(gnss_coords[2])
            xacro_command.append(' ')

    # Robot State Publisher node
    rsp_name = camera_name_val + '_state_publisher'
    rsp_node = Node(
        condition=IfCondition(publish_urdf),
        package='robot_state_publisher',
        namespace=namespace_val,
        executable='robot_state_publisher',
        name=rsp_name,
        output='screen',
        parameters=[{
            'robot_description': Command(xacro_command)
        }]
    )
    return_array.append(rsp_node)

    return return_array


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                'camera_name',
                default_value=TextSubstitution(text='zed'),
                description='The name of the camera. It can be different from the camera model and it will be used as node `namespace`.'),
            DeclareLaunchArgument(
                'camera_model',
                description='[REQUIRED] The model of the camera. Using a wrong camera model can disable camera features.',
                choices=['zed', 'zedm', 'zed2', 'zed2i', 'zedx', 'zedxm', 'virtual', 'zedxonegs', 'zedxone4k']),
            DeclareLaunchArgument(
                'namespace',
                default_value='',
                description='The namespace of the node. If empty (default) the camera name is used.'),
            DeclareLaunchArgument(
                'publish_urdf',
                default_value='true',
                description='Enable URDF processing and starts Robot State Published to propagate static TF.',
                choices=['true', 'false']),
            DeclareLaunchArgument(
                'xacro_path',
                default_value=TextSubstitution(text=default_xacro_path),
                description='Path to the camera URDF file as a xacro file.'),            
            DeclareLaunchArgument(
                'enable_gnss',
                default_value='false',
                description='Enable GNSS fusion to fix positional tracking pose with GNSS data from messages of type `sensor_msgs::msg::NavSatFix`. The fix topic can be customized in `common_stereo.yaml`.',
                choices=['true', 'false']),
            DeclareLaunchArgument(
                'gnss_antenna_offset',
                default_value='[]',
                description='Position of the GNSS antenna with respect to the mounting point of the ZED camera. Format: [x,y,z]'),
            DeclareLaunchArgument(
                'custom_baseline',
                default_value='0.0',
                description='Distance between the center of ZED X One cameras in a custom stereo rig.'),
            OpaqueFunction(function=launch_setup)
        ]
    )