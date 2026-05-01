# yolo_related 
This repository provides a ROS2-based system for object detection using the YOLOv8 model and the ZED stereo camera. It uses yolov8_ros package from [the repo called GD-yolo_ros](https://github.com/GreenDinoBV/GD-yolo_ros) for object detection in the ROS2 environment.

## INSTALLATION
-  Clone the repository under the src file of workspace
```git clone --recursive https://github.com/GreenDinoBV/GD-Perception.git```

-  Clone the repository under the src file of workspace
```git clone --recursive https://github.com/GreenDinoBV/GD-yolo_ros.git```

-  Navigate back to the main workspace directory
```cd ~/<main_dir>```

- Update the package lists
```sudo apt update```

- Install dependencies using rosdep
```rosdep install --from-paths src --ignore-src -r -y```

- Build the workspace
```colcon build --symlink-install```

- Add the setup.bash file to the bashrc for automatic sourcing
```echo source ~/yolov8_ws/install/setup.bash >> ~/.bashrc```

- Source the bashrc to apply changes immediately
```source ~/.bashrc```

**Note:** Change the "dataset_dir" parameter in the "~/.config/Ultralytics/settings.yaml" file to the location where you intend to store your datasets. For instance, set "dataset_dir" to "/<absolute_path>/yolov8_ws/src/custom_yolov8_model_generation/datasets". This allows you to use the relative path in the [config file where you specify the dataset path.](#config)

## PACKAGES
### 1. [yolov8_ros](https://github.com/GreenDinoBV/GD-yolo_ros/tree/gd-main/yolov8_ros): This package enables you to perform object detection and tracking, instance segmentation, and human pose estimation using ROS 2 wrap for Ultralytics YOLOv8.

### 2. custom_yolov8_model_generation: This package is responsible for creating a new YOLOv8 model with a custom dataset.
### config
- **pothole_dataset_config.yaml:** It contains the parameters for training a YOLOv8 model on a custom dataset. It defines paths for training anf validation images and assigns a numerical identifier to the class name which is 'pothole' for this one.
    - **path:** Specifies the path to the dataset.
    - **train**: Specifies the subdirectory where training images are located. 
    - **val:** Specifies the subdirectory where validation images are located. 
    - **names:** Associates numerical class labels with their corresponding names. In this example, class 0 is labeled as 'pothole'.
        - **0:** 'pothole' 
- **yolov8_training_config.yaml:** It contains the parameters for training a new YOLOv8 model in the ROS2 framework." is this a true statment for yaml fıle
    - **model:** Specifies the filename ("yolov8n.pt") of the YOLOv8 model to be used.
    - **yaml_file:** Specifies the filename ("pothole_dataset_config.yaml") of another YAML file containing configuration details for the dataset.
    - **image_size:** Specifies the size of the input images for the model.
    - **epochs:** Specifies the number of training epochs for model training.
    - **batch_size:** Specifies the batch size used during training.
    - **model_name:** Specifies a custom name for the new YOLOv8 model.
### script
- **train.py:** This node is responsible for training a custom YOLOv8 model. 
### dataset
- This folder is created for the placement of custom datasets.(To install example pothole dataset and more detail, you can check https://learnopencv.com/train-yolov8-on-custom-dataset/ website.)
### launch
- **custom_yolov8_model.launch.py:** This launch file configures and initiates the training process for a custom YOLOv8 model by specifying the relevant YAML configuration.
### models
- This folder is created for the placement of custom or default YOLOv8 models.

### 3. object_detection_launch: This package responsible for bringing up the entire system with different configurations.
### config
- **yolo_config.yaml:** It contains the parameters for configuring "yolov8_node".
    - **yolo/default_params:**
        - **model:** Specifies the YOLOv8 model to be used. Model needs to be located under the "custom_yolov8_model_generation/models" folder.
        - **tracker:** "bytetrack.yaml"
        - **device:** Specifies the device (GPU or CPU) to be used for YOLOv8. It has a default value of "cuda:0", indicating GPU usage. You can change this to "cpu" or another GPU identifier.
        - **enable:** Specifies whether to start YOLOv8 enabled.
        - **threshold:** Sets the minimum probability threshold for a detection to be published.
        - **image_reliability:** Sets the reliability QoS (Quality of Service) of the input image topic. You can choose from "0" (system default), "1" (Reliable), or "2" (Best Effort).
        - **depth_image_reliability:** Sets the reliability QoS of the input depth image topic. You can choose from "0" (system default), "1" (Reliable), or "2" (Best Effort).
        - **depth_info_reliability:** Sets the reliability QoS of the input depth info topic. You can choose from "0" (system default), "1" (Reliable), or "2" (Best Effort).
        - **depth_image_units_divisor:** Sets the divisor used to convert the raw depth image values into meters.
        - **maximum_detection_threshold:** Sets the maximum detection threshold in the z-axis. 
    - **yolo/remapped_topics:**
        - **input_image_topic:** Specifies the name of the input image topic.
        - **input_depth_info_topic:** Specifies the name of the input image topic.
        - **input_depth_topic:**  Specifies the name of the input depth topic.
        - **target_frame:** Specifies the target frame to which the 3D boxes should be transformed.

### launch
- **yolov8.launch.py:** This file is copied from "yolov8_ros/yolov8_bringup/launch".The difference is that it contains a YAML file named ("yolo_config.yaml").
- **yolov8_3d.launch.py:** This file is copied from "yolov8_ros/yolov8_bringup/launch". The difference is that it contains a YAML file named ("yolo_config.yaml"). In this way, parameters can be changed from the YAML file without touching the launch file.
- **detection.launch.xml:** This file brings the This file fetches the 2D object detection module ("yolov8.launch.py") or the 3D object detection module ("yolov8_3d.launch.py") according to the "enable_3d_object_detection" parameter. It also brings RVIZ visualization.
### rviz
- **yolov8.rviz:** This is a RVIZ configuration file for visualization of object detection using YOLOv8.

### 4. video_tools: <span style="color:red">This requires the <a href="https://github.com/stereolabs/zed-ros2-wrapper">zed-ros2-wrapper</a> package.</span> This package provides a versatile solution for working with camera data, offering options to replay recorded rosbag files, stream live data from the ZED camera, and process ZED camera SVO files.
### launch
- **rosbag2_play.launch.py:** This launch file replays previously recorded rosbag files. It has two arguements that is given below. 
    -  **rosbag2_path:** The directory path where the rosbag2 file is located.
    -  **rosbag2_topics:** This used for specifying the topics to play from the ros2 bag file during playback.
- **zed_camera_video.launch.xml:** This launch file streams previously recorded ZED videos or live ZED data.
    - **svo_path:** The directory path where the svo file is located. If this argument is left empty, the ZED camera will stream live data.
- **video_source_selection.launch.xml:** This launch file combines two different launch functionalities: "rosbag2_play.launch.py" and "zed_camera_video.launch.xml". It provides a flexible solution for selecting the video source based on user preference or application requirements.
    - **video_source:** Users can set this argument to specify the video source. Options are "rosbag2" and "camera".
    - **rosbag2_path:** Users need to be set this argument to provide the path to the rosbag file when the video source is set to "rosbag2". This argument is same with the "rosbag2_play.launch.py".
    - **rosbag2_topics:** When the video source is set to "rosbag2" users need to set the argument if they want to replay only certain topics. This argument is the same as "rosbag2_play.launch.py".
    - **svo_path:** Users need to be set this argument to provide the path to the svo file when the video source is set to "camera". This argument is same with the "zed_camera_video.launch.xml".

### 5. bringup_launch: The main goal of this launch file is to bringup the entire system.
### config
- **bringup_config.yaml:** It contains the parameters for configuring "bringup.launch.py".
    - **video_tools:** The parameters in this section are related to video source selection and initialization file. These parameters align with those found in the "video_source_selection.launch.xml" launch file within the "video_tools" package. This similarity arises because the "bringup.launch.xml" file includes and utilizes the "video_source_selection.launch.xml" file for configuring the video source selection.
        - **video_source:** Users can set this argument to specify the video source. Options are "rosbag2" and "camera".
        - **rosbag2_path:** Users need to be set this argument to provide the path to the rosbag file when the video source is set to "rosbag2". This argument is same with the "rosbag2_play.launch.py".
        - **rosbag2_topics:** When the video source is set to "rosbag2" users need to set the argument if they want to replay only certain topics. This argument is the same as "rosbag2_play.launch.py".
        - **svo_path:** Users need to be set this argument to provide the path to the svo file when the video source is set to "camera". This argument is same with the "zed_camera_video.launch.xml".
    - **object_detection_launch:** The parameters in this section are related to object detection module and initialization file. These parameters align with those found in the "detection.launch.xml" launch file within the "object_detection_launch" package. This similarity arises because the "bringup.launch.xml" file includes and utilizes the "detection.launch.xml" file for configuring the object detection module.
        - **enable_3d_object_detection:** Specifies whether to enable 3D object detection.
        - **rviz:** Specifies whether to launch RViz for visualization. 

        - **model:** Specifies the YOLOv8 model to be used. Model needs to be located under the "custom_yolov8_model_generation/models" folder.
        - **tracker:** "bytetrack.yaml"
        - **device:** Specifies the device (GPU or CPU) to be used for YOLOv8. It has a default value of "cuda:0", indicating GPU usage. You can change this to "cpu" or another GPU identifier.
        - **enable:** Specifies whether to start YOLOv8 enabled.
        - **threshold:** Sets the minimum probability threshold for a detection to be published.
        - **image_reliability:** Sets the reliability QoS (Quality of Service) of the input image topic. You can choose from "0" (system default), "1" (Reliable), or "2" (Best Effort).
        - **depth_image_reliability:** Sets the reliability QoS of the input depth image topic. You can choose from "0" (system default), "1" (Reliable), or "2" (Best Effort).
        - **depth_info_reliability:** Sets the reliability QoS of the input depth info topic. You can choose from "0" (system default), "1" (Reliable), or "2" (Best Effort).
        - **depth_image_units_divisor:** Sets the divisor used to convert the raw depth image values into meters.
        - **maximum_detection_threshold:** Sets the maximum detection threshold in the z-axis. 

        - **input_image_topic:** Specifies the name of the input image topic.
        - **input_depth_info_topic:** Specifies the name of the input image topic.
        - **input_depth_topic:**  Specifies the name of the input depth topic.
        - **target_frame:** Specifies the target frame to which the 3D boxes should be transformed.
### launch
- **bringup.launch.py:** This launch file is to bringup the entire system by dynamically configuring and launching two distinct components: video source selection and object detection. The YAML file ("bringup_config.yaml") is used to configure.

### 6. yolo_autoware_feedback: This includes multiple packages. For details, check the README under the related packages.

## USAGE
### Creating New YOLOv8 Model with Custom Dataset
1. Create new YAML file under the "custom_yolov8_model_generation/config" folder. You can check the "pothole_dataset_config.yaml" as an example.
2. Modify the "yolov8_training_config.yaml" YAML file under the "custom_yolov8_model_generation/config" according to new dataset.
3. Run the below command to start training. Keep in mind that the duration of the training may vary based on factors such as the size of your dataset, your computer's capabilities, and other relevant considerations.

     ```ros2 launch custom_yolov8_model_generation custom_yolov8_model.launch.py```

### Bringup Object Detection Module
1. Modify the "yolo_config.yaml" YAML file under the "object_detection_launch/config" according to needs. For instance, if you plan to use a different model, you will need to update it in this YAML file.
2. Run the below command to bringup object detection module with RVIZ visaulization.
    - For 2D object detection: ```ros2 launch object_detection_launch detection.launch.xml enable_3d_object_detection:=false```
    - For 3D object detection: ```ros2 launch object_detection_launch detection.launch.xml enable_3d_object_detection:=true```

### Select Video Source
<span style="color:red">Note: This requires the <a href="https://github.com/stereolabs/zed-ros2-wrapper">zed-ros2-wrapper</a> package.</span>
There are three different options for video source.
1. Replay from previously recorded ROS2 bags

    ```ros2 launch video_tools video_source_selection.launch.xml video_source:='rosbag2' rosbag2_path:='/<absolute_path_for_ros2bag>/<ros2bag_name>.bag'```
    
    If you want to see only certain topics, not all topics:

    ```ros2 launch video_tools video_source_selection.launch.xml video_source:='rosbag2' rosbag2_path:='/<absolute_path_for_ros2bag>/<ros2bag_name>.bag' rosbag2_topics:='/topic1 /topic2'```

2. Replay from previously recorded SVO files

    ```ros2 launch video_tools video_source_selection.launch.xml video_source:='camera' svo_path:='/<absolute_path_for_svo>/<svo_name>.bag'```
3. Live stream from the ZED camera

    ```ros2 launch video_tools video_source_selection.launch.xml video_source:='camera'```
### Bringup the Whole System
1. Modify the "bringup_config.yaml" YAML file under the "bringup_launch/config" according to needs.
2. Run the below command to bringup the whole system that includes "video_source_selection.launch.xml" from "video_tools" package and "detection.launch.xml" from "object_detection_launch".

    ```ros2 launch bringup_launch bringup.launch.py```


### TODO
- [ ] cleanup. Separate packages, rename, review etc (Kübra)