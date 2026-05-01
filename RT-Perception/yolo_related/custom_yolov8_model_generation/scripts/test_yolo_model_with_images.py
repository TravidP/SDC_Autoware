#!/bin/env python3

import os
import sys
from ament_index_python.packages import get_package_share_directory
package_share_directory = get_package_share_directory('custom_yolov8_model_generation')
ultralytics_path = os.path.join(package_share_directory, 'lib', 'ultralytics')
sys.path.insert(0, ultralytics_path)

import cv2
import torch
import easyocr
import numpy as np
import matplotlib.pyplot as plt
from ultralytics import YOLO

# This function is find the color of the detected traffic light
def detect_traffic_light_color(roi):
    roi2 = cv2.cvtColor(roi, cv2.COLOR_BGR2RGB)
    roi_hsv = cv2.cvtColor(roi2, cv2.COLOR_RGB2HSV)

    lower_red = np.array([0, 100, 100]) 
    upper_red = np.array([10, 255, 255])
    lower_yellow = np.array([20, 100, 100])
    upper_yellow = np.array([30, 255, 255])
    lower_green = np.array([60, 100, 100])
    upper_green = np.array([90, 255, 255])

    mask_red = cv2.inRange(roi_hsv, lower_red, upper_red)
    mask_yellow = cv2.inRange(roi_hsv, lower_yellow, upper_yellow)
    mask_green = cv2.inRange(roi_hsv, lower_green, upper_green)

    red_pixel_count = cv2.countNonZero(mask_red)
    yellow_pixel_count = cv2.countNonZero(mask_yellow)
    green_pixel_count = cv2.countNonZero(mask_green)

    max_count = max(red_pixel_count, yellow_pixel_count, green_pixel_count)
    if max_count == red_pixel_count:
        return "red"
    elif max_count == yellow_pixel_count:
        return "yellow"
    elif max_count == green_pixel_count:
        return "green"

    return None

def show_output(images):
    image = np.vstack(images)
    fig = plt.figure()
    ax = plt.Axes(fig, [0., 0., 1., 1.])
    ax.set_axis_off()
    fig.add_axes(ax)
    fig.set_size_inches((5,15))
    ax.imshow(image[...,::-1])
    plt.show()

image = "/home/greendinokubra/yolov8_ws/src/object_detection_with_custom_model/custom_yolov8_model_generation/datasets/image/crosswalk.webp"
image_cv2 = cv2.imread(image)
reader = easyocr.Reader(['en'], gpu=True)

# Extended yolov8n
merged_yolo_model = YOLO('../../ultralytics/ultralytics/cfg/models/v8/yolov8n-2xhead.yaml', task="detect").load('yolov8n.pt')
merged_yolo_model.state_dict().keys()
state_dict = torch.load("yolov8n_lp.pth")
state_dict.keys()
merged_yolo_model.load_state_dict(state_dict, strict=False)
result_merged = merged_yolo_model.predict(image)[0]

# Coco yolov8n
coco_yolo_model = YOLO("yolov8n.pt")
coco_yolo_model.model.names = {k:k for k in coco_yolo_model.names.keys()}
result_coco = coco_yolo_model.predict(image)[0]

# Model with only new classes
model_last_layer = YOLO("best.pt")
model_last_layer.model.names = {k:k for k in coco_yolo_model.names.keys()}
result_last_layer = model_last_layer.predict(image)[0]


detected_classes = result_merged.boxes.cls.tolist()
for count, detected_class in enumerate(detected_classes):
    # detect_traffic_light_color function is applied, if it traffic light object
    if (int(detected_class) == 9):
        detected_box = result_merged.boxes.xyxy[count].tolist()
        objects = image_cv2[int(detected_box[1]):int(detected_box[3]), int(detected_box[0]):int(detected_box[2])]
        traffic_light_color = detect_traffic_light_color(objects)
        print(f"Detected with color: {traffic_light_color}")
        plt.imshow(objects)
        plt.axis('off')
        plt.show()

    # OCR is applied, if it is speed sign object
    if (int(detected_class) == 80):
        print("Detected boxes: ")
        detected_box = result_merged.boxes.xyxy[count].tolist()
        objects = image_cv2[int(detected_box[1]):int(detected_box[3]), int(detected_box[0]):int(detected_box[2])]
        result = reader.readtext(objects)
        for (bbox, text, prob) in result:
            print(f'Text: {text}, Probability: {prob}')
        plt.imshow(objects)
        plt.axis('off')
        plt.show()


show_output([result_coco.plot(), result_last_layer.plot(), result_merged.plot()])
