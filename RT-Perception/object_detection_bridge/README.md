# Object detection bridge

This package bridges object detection data from ZED cameras and YOLOv8 models to Autoware, seamless integration of object detection results into the Autoware. The package also supports point cloud preprocessing for enhanced perception tasks.

## include
- **object_detection_zed_bridge.hpp**
- **object_detection_yolo_bridge.hpp**

## src
- **object_detection_zed_bridge.cpp:** This is a node that bridges object detection data from a ZED camera to Autoware, enabling integration and use of detected objects within the Autoware.
    - **Subscriber:** /zed/zed_node/obj_det/objects (zed_interfaces::msg::ObjectsStamped)
        - Source of object detection data from the ZED camera.
    - **Publisher:** /perception/object_recognition/objects (autoware_perception_msgs::msg::PredictedObjects)
        - Output topic where converted Autoware-compatible objects are published.
- **object_detection_yolo_bridge.cpp:** This is a node that bridge object detection data from YOLOv8 to Autoware, enabling integration of detected objects into Autoware. This node translates and transforms the YOLOv8 object detection data into the format required by Autoware, allowing the detected objects to be utilized effectively.
    - **Subscriber:** /yolo/detections_3d (yolov8_msgs::msg::DetectionArray)
        - Receives YOLOv8 object detection messages containing 3D bounding box data.
    - **Publisher:** /perception/object_recognition/objects (autoware_perception_msgs::msg::PredictedObjects)
        - Output topic where converted Autoware-compatible objects are published.
## launch
- **gd_perception.launch.xml:** Includes the object_detection_yolo_bridge node and gd_pointcloud_preprocessing launch file.

## USAGE
1. Run the below command.

     ```ros2 launch greendino_perception gd_perception.launch.xml```

## TODO

- [ ] update lauch file to have a choice between which executable to launch
- [ ] run short review
- [ ] add description
- [ ] add to perception launch?
