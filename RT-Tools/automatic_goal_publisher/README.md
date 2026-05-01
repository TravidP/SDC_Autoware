# automatic_goal_publisher 
This package is responsible for sending goals automatically.

## config
- This folder contains YAML file that includes the list of goal. User can be change these goal for a new map or create a new YAML file.
## launch
- **automatic_goal_publisher.launch.xml:** This launch file starts the goal publisher node. It includes two parameters:
    - **filename:** the path of the yaml file that includes goal list.
    - **distance_to_goal_upper_limit:** The maximum allowable distance between the vehicle and the current goal to assign a new goal.
## src
- **publish_goal.cpp:** This source file includes the implementation of the goal publisher, which reads goals from a specified YAML file and publishes them with a certain condition.

## USAGE
1. Open the autoware simulation.
2. Change the goals inside of the YAML file or create a new one.
2. Run the below command to start.

     ```ros2 launch automatic_goal_publisher automatic_goal_publisher.launch.xml filename:=/<absolute_path>/GD-Tools/automatic_goal_publisher/config/goal_list.yaml distance_to_goal_upper_limit:=20.0```