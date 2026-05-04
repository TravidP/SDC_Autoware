## 1. System Architecture & Launch Sequence

The `twizy_perception.launch.xml` acts as the primary entry point for the perception pipeline. It orchestrates the following components:

* **Direct Node Execution:** 
  * `object_detection_yolo_bridge`: Bridges external YOLO detections with Autoware-standard message formats.
* **Nested Launch Files (via `twizy_bringup.launch.py`):**
  * `detection.launch.xml`: Deploys YOLOv8 for 2D object detection, tracking, and 3D bounding box generation using depth data.
  * `yolo_feedback.launch.py`: Handles high-level semantic recognition (Traffic light colors and speed limit OCR).
  * `nearest_traffic_light.launch.xml`: Provides map-based context for traffic signal filtering.

---

## 2. Component Functional Breakdown

### A. Object Detection & 3D Transformation (YOLOv8 Suite)
Processes raw RGB images and depth information to identify obstacles, traffic lights, and speed signs. It transforms 2D pixels into 3D spatial coordinates.

### B. Message Format Bridging (`object_detection_yolo_bridge`)
Acts as the "translator" between the vision pipeline and the planning system.
* **Coordinate Transformation:** Converts detections from `base_link` (vehicle frame) to `map` (global frame) via TF.
* **Standardization:** Encapsulates raw data into `autoware_auto_perception_msgs/PredictedObjects`.
* **Trajectory Estimation:** Generates short-term motion predictions for detected objects.

### C. Map-based Context (`nearest_traffic_light`)
* **Map Parsing:** Reads `.osm` files (Lanelet2) to extract traffic light locations.
* **Proximity Filter:** Compares vehicle Odometry with map data to output the ID of the closest traffic light (within a 25-meter radius).

### D. Semantic Recognition (`yolo_feedback`)
* **Traffic Lights:** Combines the "Closest Light ID" with the YOLO bounding box. It uses **HSV color space masking** (Red/Yellow/Green) to determine the current signal state.
* **Speed Signs:** Extracts the region of interest (ROI) for speed limit signs and utilizes **OCR (pytesseract)** to read numeric values (e.g., 10, 20, 30 km/h).

---

## 3. Communication Interface (Topics)

| Node | Subscribed Topics | Published Topics |
| :--- | :--- | :--- |
| **object_detection_yolo_bridge** | `/yolo/detections_3d` | `/perception/object_recognition/objects` |
| **nearest_traffic_light** | `/localization/kinematic_state` | `/yolo_autoware_tools/closest_traffic_light_id` |
| **yolo_feedback** | 1. `/yolo/specific_image_detections`<br>2. `/yolo/spesific_detections_3d`<br>3. `/yolo_autoware_tools/closest_traffic_light_id` | 1. `/perception/traffic_light_recognition/traffic_signals`<br>2. `/planning/scenario_planning/max_velocity` |

> *Note: Low-level YOLOv8 nodes also subscribe to sensor-level topics like `/image_raw` and `/depth_image`.*

---

## 4. System Logic & Data Flow Diagram

The following diagram illustrates the relationship between sensors, perception nodes, and the downstream planning modules.

```mermaid
graph TD
    %% Sensors
    subgraph Sensors
        CAM[Camera /image_raw]
        DEP[Depth Camera /depth_image]
        ODO[Odometry /kinematic_state]
    end

    %% Perception Nodes
    subgraph Perception_Module
        YOLO[YOLOv8 Detection & 3D Projection]
        BRIDGE[object_detection_yolo_bridge]
        MAP_TL[nearest_traffic_light]
        FEEDBACK[yolo_feedback OCR & HSV]
    end

    %% Map Data
    MAP_DATA[(Lanelet2 HD Map)]

    %% Downstream
    subgraph Planning_Control
        AVOID[Object Avoidance]
        TL_LOGIC[Traffic Light Reaction]
        SPEED_CTRL[Dynamic Speed Control]
    end

    %% Connections
    CAM --> YOLO
    DEP --> YOLO
    ODO --> MAP_TL
    MAP_DATA --> MAP_TL

    YOLO -- "/yolo/detections_3d" --> BRIDGE
    BRIDGE -- "/perception/object_recognition/objects" --> AVOID

    MAP_TL -- "Closest TL ID" --> FEEDBACK
    YOLO -- "Crop / ROIs" --> FEEDBACK
    
    FEEDBACK -- "/perception/traffic_light_recognition/traffic_signals" --> TL_LOGIC
    FEEDBACK -- "/planning/scenario_planning/max_velocity" --> SPEED_CTRL

    %% Styling
    style BRIDGE fill:#f9f,stroke:#333,stroke-width:2px
    style FEEDBACK fill:#bbf,stroke:#333,stroke-width:2px
    style MAP_TL fill:#dfd,stroke:#333,stroke-width:2px
```

---

## 5. Downstream Impact (Business Logic)

Once the perception module publishes the processed information, the **Planning** and **Control** modules execute the following:

1.  **Dynamic Obstacle Avoidance:** Uses the 3D map-frame objects to decide whether to stop, slow down, or steer around pedestrians and vehicles.
2.  **Traffic Signal Compliance:** By mapping the vision-detected color to a specific HD Map ID, the vehicle knows exactly which stop line it must respect, enabling precise braking at intersections.
3.  **Adaptive Speed Regulation:** The system "reads" road signs. If a 15 km/h sign is detected, the OCR output overrides the default global speed limit, ensuring the vehicle operates safely according to local regulations.

***

*(注：如果你的环境由于某些原因完全不支持 Mermaid，你也可以打开网页版的 [Mermaid Live Editor](https://mermaid.live/)，将上面的 `mermaid` 代码块内容粘贴进去，就能直接生成并导出流程图的图片了。)*