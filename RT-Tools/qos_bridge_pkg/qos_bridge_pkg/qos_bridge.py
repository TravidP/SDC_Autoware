#!/usr/bin/env python3
"""
ROS2 QoS Bridge Package
A comprehensive command-line tool to bridge topics with different QoS profiles.
"""

import argparse
import sys
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy, DurabilityPolicy, LivelinessPolicy, QoSPresetProfiles
from rclpy.serialization import deserialize_message, serialize_message
from rosidl_runtime_py.utilities import get_message
import importlib


class QoSBridge(Node):
    def __init__(self, input_topic, output_topic, msg_type, input_qos, output_qos):
        super().__init__('qos_bridge')
        
        # Load message type dynamically
        try:
            msg_module = get_message(msg_type)
        except (ValueError, ModuleNotFoundError, AttributeError) as e:
            self.get_logger().error(f'Failed to load message type {msg_type}: {e}')
            raise
        
        self.msg_type = msg_module
        
        # Create subscription with input QoS
        self.subscription = self.create_subscription(
            msg_module,
            input_topic,
            self.bridge_callback,
            input_qos
        )
        
        # Create publisher with output QoS
        self.publisher = self.create_publisher(
            msg_module,
            output_topic,
            output_qos
        )
        
        self.message_count = 0
        
        self.get_logger().info(f'QoS Bridge initialized:')
        self.get_logger().info(f'  Input: {input_topic}')
        self.get_logger().info(f'    Reliability: {input_qos.reliability.name}')
        self.get_logger().info(f'    Durability: {input_qos.durability.name}')
        self.get_logger().info(f'    History: {input_qos.history.name} (depth: {input_qos.depth})')
        self.get_logger().info(f'  Output: {output_topic}')
        self.get_logger().info(f'    Reliability: {output_qos.reliability.name}')
        self.get_logger().info(f'    Durability: {output_qos.durability.name}')
        self.get_logger().info(f'    History: {output_qos.history.name} (depth: {output_qos.depth})')
    
    def bridge_callback(self, msg):
        self.publisher.publish(msg)
        self.message_count += 1
        if self.message_count == 1:
            self.get_logger().info('First message received and bridged!')
        if self.message_count % 100 == 0:
            self.get_logger().info(f'Bridged {self.message_count} messages')


def parse_qos_args(args, prefix):
    """Parse QoS arguments with given prefix (input/output)"""
    qos = QoSProfile(depth=10)
    
    # Reliability
    reliability = getattr(args, f'{prefix}_reliability')
    if reliability == 'reliable':
        qos.reliability = ReliabilityPolicy.RELIABLE
    elif reliability == 'best_effort':
        qos.reliability = ReliabilityPolicy.BEST_EFFORT
    
    # Durability
    durability = getattr(args, f'{prefix}_durability')
    if durability == 'transient_local':
        qos.durability = DurabilityPolicy.TRANSIENT_LOCAL
    elif durability == 'volatile':
        qos.durability = DurabilityPolicy.VOLATILE
    
    # History
    history = getattr(args, f'{prefix}_history')
    if history == 'keep_last':
        qos.history = HistoryPolicy.KEEP_LAST
        qos.depth = getattr(args, f'{prefix}_depth')
    elif history == 'keep_all':
        qos.history = HistoryPolicy.KEEP_ALL
    
    # Liveliness
    liveliness = getattr(args, f'{prefix}_liveliness')
    if liveliness == 'automatic':
        qos.liveliness = LivelinessPolicy.AUTOMATIC
    elif liveliness == 'manual_by_topic':
        qos.liveliness = LivelinessPolicy.MANUAL_BY_TOPIC
    
    return qos


def main():
    parser = argparse.ArgumentParser(
        description='Bridge ROS2 topics with different QoS profiles',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Bridge from BEST_EFFORT to RELIABLE
  qos_bridge /input_topic /output_topic sensor_msgs/msg/Imu \\
    --input-reliability best_effort \\
    --output-reliability reliable

  # Full QoS specification
  qos_bridge /imu/data /imu/reliable sensor_msgs/msg/Imu \\
    --input-reliability best_effort \\
    --input-history keep_last \\
    --input-depth 5 \\
    --output-reliability reliable \\
    --output-history keep_last \\
    --output-depth 1
        """
    )
    
    # Positional arguments
    parser.add_argument('input_topic', help='Input topic name')
    parser.add_argument('output_topic', help='Output topic name')
    parser.add_argument('message_type', help='Message type (e.g., sensor_msgs/msg/Imu)')
    
    # Preset profiles
    parser.add_argument('--input-preset',
                       choices=['sensor_data', 'default', 'system_default', 'services_default', 'parameters', 'parameter_events'],
                       help='Use a QoS preset profile for input (overrides individual settings)')
    parser.add_argument('--output-preset',
                       choices=['sensor_data', 'default', 'system_default', 'services_default', 'parameters', 'parameter_events'],
                       help='Use a QoS preset profile for output (overrides individual settings)')
    
    # Input QoS arguments
    parser.add_argument('--input-reliability', 
                       choices=['reliable', 'best_effort'],
                       default='best_effort',
                       help='Input reliability policy (default: best_effort)')
    parser.add_argument('--input-durability',
                       choices=['transient_local', 'volatile'],
                       default='volatile',
                       help='Input durability policy (default: volatile)')
    parser.add_argument('--input-history',
                       choices=['keep_last', 'keep_all'],
                       default='keep_last',
                       help='Input history policy (default: keep_last)')
    parser.add_argument('--input-depth',
                       type=int,
                       default=10,
                       help='Input history depth (default: 10)')
    parser.add_argument('--input-liveliness',
                       choices=['automatic', 'manual_by_topic'],
                       default='automatic',
                       help='Input liveliness policy (default: automatic)')
    
    # Output QoS arguments
    parser.add_argument('--output-reliability',
                       choices=['reliable', 'best_effort'],
                       default='reliable',
                       help='Output reliability policy (default: reliable)')
    parser.add_argument('--output-durability',
                       choices=['transient_local', 'volatile'],
                       default='volatile',
                       help='Output durability policy (default: volatile)')
    parser.add_argument('--output-history',
                       choices=['keep_last', 'keep_all'],
                       default='keep_last',
                       help='Output history policy (default: keep_last)')
    parser.add_argument('--output-depth',
                       type=int,
                       default=10,
                       help='Output history depth (default: 10)')
    parser.add_argument('--output-liveliness',
                       choices=['automatic', 'manual_by_topic'],
                       default='automatic',
                       help='Output liveliness policy (default: automatic)')
    
    args = parser.parse_args()
    
    # Parse QoS profiles
    if args.input_preset:
        preset_map = {
            'sensor_data': QoSPresetProfiles.SENSOR_DATA.value,
            'default': QoSPresetProfiles.DEFAULT.value,
            'system_default': QoSPresetProfiles.SYSTEM_DEFAULT.value,
            'services_default': QoSPresetProfiles.SERVICES_DEFAULT.value,
            'parameters': QoSPresetProfiles.PARAMETERS.value,
            'parameter_events': QoSPresetProfiles.PARAMETER_EVENTS.value,
        }
        input_qos = preset_map[args.input_preset]
    else:
        input_qos = parse_qos_args(args, 'input')
    
    if args.output_preset:
        preset_map = {
            'sensor_data': QoSPresetProfiles.SENSOR_DATA.value,
            'default': QoSPresetProfiles.DEFAULT.value,
            'system_default': QoSPresetProfiles.SYSTEM_DEFAULT.value,
            'services_default': QoSPresetProfiles.SERVICES_DEFAULT.value,
            'parameters': QoSPresetProfiles.PARAMETERS.value,
            'parameter_events': QoSPresetProfiles.PARAMETER_EVENTS.value,
        }
        output_qos = preset_map[args.output_preset]
    else:
        output_qos = parse_qos_args(args, 'output')
    
    # Initialize ROS2
    rclpy.init()
    
    try:
        bridge = QoSBridge(
            args.input_topic,
            args.output_topic,
            args.message_type,
            input_qos,
            output_qos
        )
        
        rclpy.spin(bridge)
        
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f'Error: {e}', file=sys.stderr)
        return 1
    finally:
        if rclpy.ok():
            rclpy.shutdown()
    
    return 0


if __name__ == '__main__':
    sys.exit(main())