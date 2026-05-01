import easyocr
import cv2
import message_filters
import numpy as np
import pytesseract

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile
from rclpy.qos import QoSHistoryPolicy
from rclpy.qos import QoSDurabilityPolicy
from rclpy.qos import QoSReliabilityPolicy
from cv_bridge import CvBridge

from std_msgs.msg import Int32
from sensor_msgs.msg import Image
from autoware_perception_msgs.msg import TrafficLightGroupArray
from autoware_perception_msgs.msg import TrafficLightGroup
from autoware_perception_msgs.msg import TrafficSignalElement
from tier4_planning_msgs.msg import VelocityLimit
from yolo_autoware_msgs.msg import DetectionHeader

class YoloFeedback(Node):
    def __init__(self) -> None:
        super().__init__("yolo_feedback")
        # Declare parameters
        self.declare_parameter('lower_red', [0, 120, 70])
        self.declare_parameter('upper_red', [10, 255, 255])
        self.declare_parameter('lower_yellow', [20, 100, 100])
        self.declare_parameter('upper_yellow', [30, 255, 255])
        self.declare_parameter('lower_green', [60, 100, 100])
        self.declare_parameter('upper_green', [90, 255, 255])
        self.declare_parameter('lateral_distance_lower_bound', 1.8)
        self.declare_parameter('lateral_distance_upper_bound', 2.1)
        self.declare_parameter('height_lower_bound', 0.4)
        self.declare_parameter('height_upper_bound', 1.2)

        self.LOWER_RED = np.array(self.get_parameter('lower_red').get_parameter_value().integer_array_value, dtype=np.uint8)
        self.UPPER_RED = np.array(self.get_parameter('upper_red').get_parameter_value().integer_array_value, dtype=np.uint8)
        self.LOWER_YELLOW = np.array(self.get_parameter('lower_yellow').get_parameter_value().integer_array_value, dtype=np.uint8)
        self.UPPER_YELLOW = np.array(self.get_parameter('upper_yellow').get_parameter_value().integer_array_value, dtype=np.uint8)
        self.LOWER_GREEN = np.array(self.get_parameter('lower_green').get_parameter_value().integer_array_value, dtype=np.uint8)
        self.UPPER_GREEN = np.array(self.get_parameter('upper_green').get_parameter_value().integer_array_value, dtype=np.uint8)

        self.lateral_distance_lower_bound = self.get_parameter('lateral_distance_lower_bound').get_parameter_value().double_value
        self.lateral_distance_upper_bound = self.get_parameter('lateral_distance_upper_bound').get_parameter_value().double_value
        self.height_lower_bound = self.get_parameter('height_lower_bound').get_parameter_value().double_value
        self.height_upper_bound = self.get_parameter('height_upper_bound').get_parameter_value().double_value

        # Constants for subscriber QoS profile
        SUBS_QOS_PROFILE = QoSProfile(
            reliability=QoSReliabilityPolicy.RELIABLE,
            history=QoSHistoryPolicy.KEEP_LAST,
            durability=QoSDurabilityPolicy.VOLATILE,
            depth=1
        )

        # Publisher
        self._traffic_light_status_pub = self.create_publisher(TrafficLightGroupArray, 
            "/perception/traffic_light_recognition/traffic_signals", 1)
        self._max_vel_pub = self.create_publisher(VelocityLimit, 
            "/planning/scenario_planning/max_velocity", 1)

        # Subscriber
        self.image_sub = message_filters.Subscriber(
            self, Image, "/yolo/specific_image_detections",
            qos_profile=SUBS_QOS_PROFILE)
        self.detect_sub = message_filters.Subscriber(
            self, DetectionHeader, "/yolo/spesific_detections_3d",
            qos_profile=SUBS_QOS_PROFILE)
        self.traffic_light_id_sub = self.create_subscription(
            Int32, "/yolo_autoware_tools/closest_traffic_light_id", self.traffic_light_id_cb,
            qos_profile=SUBS_QOS_PROFILE)

        self._synchronizer = message_filters.ApproximateTimeSynchronizer(
            (self.image_sub, self.detect_sub), 10, 0.5)
        self._synchronizer.registerCallback(self.on_detections)

        self.cv_bridge = CvBridge()
        self.closest_traffic_light_id = Int32()
        self.reader = easyocr.Reader(['en'], gpu=True)
    
    def bbox_xyxy(self, bbox_xywh):
        center_x = bbox_xywh.center.position.x
        center_y = bbox_xywh.center.position.y
        width = bbox_xywh.size.x
        height = bbox_xywh.size.y

        top_left_x = int(center_x - width / 2)
        top_left_y = int(center_y - height / 2)
        bottom_right_x = int(center_x + width / 2)
        bottom_right_y = int(center_y + height / 2)

        return [top_left_y, bottom_right_y, top_left_x, bottom_right_x]

    def detect_traffic_light_color(self, roi):
        roi_hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)

        mask_red = cv2.inRange(roi_hsv, self.LOWER_RED, self.UPPER_RED)
        mask_yellow = cv2.inRange(roi_hsv, self.LOWER_YELLOW, self.UPPER_YELLOW)
        mask_green = cv2.inRange(roi_hsv, self.LOWER_GREEN, self.UPPER_GREEN)

        red_pixel_count = cv2.countNonZero(mask_red)
        yellow_pixel_count = cv2.countNonZero(mask_yellow)
        green_pixel_count = cv2.countNonZero(mask_green)

        max_count = max(red_pixel_count, yellow_pixel_count, green_pixel_count)
        total_relevant_pixels = sum((red_pixel_count, yellow_pixel_count, green_pixel_count))
        confidence = 0.0 if total_relevant_pixels == 0 else max_count / total_relevant_pixels

        if max_count == red_pixel_count:
            signal_color = 1
        elif max_count == green_pixel_count:
            signal_color = 3
        else:
            signal_color = 0
        return signal_color, confidence
    
    def safety_check(self, pose):
        y_coordinate = abs(pose.y)
        height = pose.z
        
        is_y_safe = self.lateral_distance_lower_bound <= y_coordinate <= self.lateral_distance_upper_bound
        is_height_safe = self.height_lower_bound <= height <= self.height_upper_bound
        
        return is_y_safe and is_height_safe

    def handle_speed_sign(self, cv_image_sign, image_msg):
        result = self.reader.recognize(cv_image_sign, allowlist='01235', decoder="wordbeamsearch")
        if result:
            first_bbox, text, prob = result[0]
            if text == '': # TODO: Fix it, not a good approach
                return
            speed_limit = float(text)
            if speed_limit in range(10, 25, 5) and prob > 0.4:
                # self.get_logger().info(f'The speed sign is {speed_limit} with {prob} probability')
                speed_sign_limit = VelocityLimit()
                speed_sign_limit.stamp = image_msg.stamp
                speed_sign_limit.max_velocity = speed_limit / 3.6
                self._max_vel_pub.publish(speed_sign_limit)
    
    def handle_speed_sign_tesseract(self, cv_image_sign, image_msg, a):
        custom_config = r'-c tessedit_char_whitelist=0125 --oem 3 --psm 8 outputbase digits'
        result = pytesseract.image_to_string(cv_image_sign, config=custom_config)
        if result != "\x0c":
            speed_limit = float(result)
            if speed_limit in range(10, 25, 5):
                # self.get_logger().info(f'The speed sign {speed_limit} with id {a.id} is {a.bbox3d.center.position} meters away')
                speed_sign_limit = VelocityLimit()
                speed_sign_limit.stamp = image_msg.stamp
                speed_sign_limit.max_velocity = speed_limit / 3.6
                self._max_vel_pub.publish(speed_sign_limit)
        
    def handle_traffic_light(self, cv_image_tf, image_msg, closest_traffic_light_id):
        traffic_signal_color, color_confidence = self.detect_traffic_light_color(cv_image_tf)
        # self.get_logger().info(f'The traffic light is {traffic_signal_color, color_confidence}')

        traffic_signals_msg = TrafficLightGroupArray()
        traffic_signal_msg = TrafficLightGroup()
        traffic_signal_msg.traffic_light_group_id = closest_traffic_light_id.data

        traffic_signal_element_msg = TrafficSignalElement()
        traffic_signal_element_msg.color = traffic_signal_color
        traffic_signal_element_msg.shape = 1
        traffic_signal_element_msg.status = 1
        traffic_signal_element_msg.confidence = color_confidence
        traffic_signal_msg.elements.append(traffic_signal_element_msg)
        traffic_signals_msg.traffic_light_groups.append(traffic_signal_msg)
        
        traffic_signals_msg.stamp = image_msg.stamp
        self._traffic_light_status_pub.publish(traffic_signals_msg)
    
    def traffic_light_id_cb(self, msg: Int32) -> None:
        self.closest_traffic_light_id = msg

    def on_detections(
        self,
        image_msg: Image,
        detection_msg: DetectionHeader,
    ) -> None:
        
        cv_image = self.cv_bridge.imgmsg_to_cv2(image_msg, desired_encoding="bgr8")

        spesific_detections_3d = detection_msg.detection
        bbox_coords = self.bbox_xyxy(spesific_detections_3d.bbox)
        cropped_image = cv_image[bbox_coords[0]:bbox_coords[1], bbox_coords[2]:bbox_coords[3]]

        if cropped_image.size > 0 and self.safety_check(spesific_detections_3d.bbox3d.center.position):
            if spesific_detections_3d.class_id == 80:
                ### EasyOCR option ###
                # self.handle_speed_sign(cropped_image, image_msg.header)
                ### pytesseract option ###
                self.handle_speed_sign_tesseract(cropped_image, image_msg.header,spesific_detections_3d)
            if spesific_detections_3d.class_id == 9:
                self.handle_traffic_light(cropped_image, image_msg.header, self.closest_traffic_light_id)

# Uses for debuggig
# import matplotlib.pyplot as plt
# plt.imshow(cv_image[bbox_coords[0]:bbox_coords[1], bbox_coords[2]:bbox_coords[3]])
# plt.axis('off')
# plt.show()

def main():
    rclpy.init()
    node = YoloFeedback()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()