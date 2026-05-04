# Perception System Launch Analysis (``twizy_perception.launch.xml``)

When the tmux script executes `ros2 launch rt_perception_launcher `twizy_perception.launch.xml` map_path:=${MAP_PATH}`, it triggers a cascade of nested launch files that bring up the 3D object detection, traffic light recognition, speed sign detection, and the bridges needed to communicate with Autoware.

## 1. Launched Files & Executed Tasks

The root launch file (``twizy_perception.launch.xml``) directly launches one C++ node and includes another Python launch file (``twizy_bringup.launch.py``). Here is the breakdown:

### A. `object_detection_yolo_bridge` (C++ Node)
* **Package**: `object_detection_bridge`
* **Task**: Acts as an adapter between the custom YOLOv8 3D detections and Autoware. It processes raw ``yolov8_msgs/DetectionArray`` messages, transforms their poses from the camera frame (`base_link`) to the `map` frame using `tf2`, assigns semantic classifications (Car, Pedestrian, Bus, Bicycle), generates a 20-step predicted static path, and publishes Autoware-compliant `PredictedObjects` messages.

### B. ``twizy_bringup.launch.py`` (Included Launch File)
This file loads parameters from ``bringup_config.yaml`` and launches three main components:

1. **``detection.launch.xml`` -> ``yolov8_3d.launch.py``**
   * **Package**: `object_detection_launch`
   * **Task**: Instantiates the core YOLO pipeline. Depending on the `enable_3d_object_detection` flag, it starts YOLOv8 for 2D/3D tracking (`yolov8_node`, `tracking_node`, `detect_3d_node`). It utilizes the custom weights (`extending_yolov8m`) and uses depth camera point clouds to estimate the 3D bounding boxes and centroids of detected targets.

2. **``nearest_traffic_light.launch.xml``**
   * **Package**: `yolo_autoware_tools`
   * **Task**: Launches the `nearest_traffic_light` node. It parses the Lanelet2 OSM map to find traffic light coordinates. By subscribing to the vehicle's odometry, it continuously calculates the Euclidean distance to all traffic lights and outputs the ID of the closest one.

3. **``yolo_feedback.launch.py``**
   * **Package**: `yolo_autoware_feedback`
   * **Task**: Launches the `yolo_feedback` Python node. This node processes specific regions of interest (ROI) from the camera feed based on YOLO's bounding boxes.
     * **Speed Signs**: Uses OCR (`pytesseract` or `easyocr`) to read speed limits on signs (class ID 80) and enforces safety checks (height and lateral distance bounds). If valid, it publishes a max velocity limit override.
     * **Traffic Lights**: Evaluates traffic light cropped images (class ID 9) using HSV color space thresholding to determine the active color (Red/Yellow/Green) and associates it with the nearest traffic light ID.

---

## 2. Subscribed and Published Topics

| Node / Component | Subscribes To | Publishes To |
| :--- | :--- | :--- |
| **YOLOv8 3D Pipeline** | ``/sensing/camera_component/rgb_raw/image_raw_color``<br>``/sensing/camera_component/depth/depth_registered``<br>``/sensing/camera_component/depth/camera_info`` | ``/yolo/detections_3d``<br>``/yolo/specific_image_detections``<br>``/yolo/spesific_detections_3d`` |
| **object_detection_yolo_bridge** | ``/yolo/detections_3d`` *(yolov8_msgs/DetectionArray)* | ``/perception/object_recognition/objects`` *(autoware_perception_msgs/PredictedObjects)* |
| **nearest_traffic_light** | ``/localization/kinematic_state`` *(nav_msgs/Odometry)* | ``/yolo_autoware_tools/closest_traffic_light_id`` *(std_msgs/Int32)* |
| **yolo_feedback** | ``/yolo/specific_image_detections`` *(sensor_msgs/Image)*<br>``/yolo/spesific_detections_3d`` *(yolo_autoware_msgs/DetectionHeader)*<br>``/yolo_autoware_tools/closest_traffic_light_id`` *(std_msgs/Int32)* | ``/perception/traffic_light_recognition/traffic_signals`` *(autoware_perception_msgs/TrafficLightGroupArray)*<br>``/planning/scenario_planning/max_velocity`` *(tier4_planning_msgs/VelocityLimit)* |

---

## 3. Flow Relationship & Subsequent Logic Diagram

The following diagram illustrates the data flow inside the perception module and how it subsequently feeds into the Autoware Planning and Perception pipelines.

```mermaid
graph TD
    %% Sensors and Inputs
    subgraph Sensors & Inputs
        RGB[Camera RGB Image]
        Depth[Camera Depth Image & Info]
        Odom[`Vehicle Kinematic State / Odometry`]
        Map[(Lanelet2 OSM Map)]
    end

    %% Core Perception
    subgraph Core YOLO Perception
        Yolo3D[YOLOv8 3D Nodes]
    end

    %% Processing & Tools
    subgraph Data Processing & Bridge
        YoloBridge[Object Detection YOLO Bridge]
        NearestTL[Nearest Traffic Light Tool]
        YoloFb[YOLO Feedback & OCR Node]
    end

    %% Autoware Output Interfaces
    subgraph Autoware Interfaces
        AutoObjects((Autoware Predicted Objects))
        AutoTL((Traffic Light Signals))
        AutoVel((Scenario Planning Max Velocity))
    end

    %% Connections
    RGB -->|`/sensing/.../image_raw_color`| Yolo3D
    Depth -->|`/sensing/.../depth_registered`| Yolo3D
    
    Yolo3D -->|`/yolo/detections_3d`| YoloBridge
    YoloBridge -->|Pose TF & Format Conv.| AutoObjects
    
    Odom -->|`/localization/kinematic_state`| NearestTL
    Map --> NearestTL
    NearestTL -->|`/yolo_autoware_tools/closest_traffic_light_id`| YoloFb
    
    Yolo3D -->|`/yolo/specific_image_detections`| YoloFb
    Yolo3D -->|`/yolo/spesific_detections_3d`| YoloFb
    
    YoloFb -->|HSV Color Matching| AutoTL
    YoloFb -->|Tesseract OCR| AutoVel

    %% Styling
    classDef node fill:#f9f9f9,stroke:#333,stroke-width:2px;
    classDef topic fill:#e1f5fe,stroke:#0288d1,stroke-width:2px;
    class AutoObjects,AutoTL,AutoVel topic;
