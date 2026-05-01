#!/bin/bash

# Find the process ID (PID) of the ros2 bag record process
PID=$(pgrep -f "ros2 bag record")
BAG_DIR=/home/$(whoami)/my_recorded_bag
TARGET_DIR=/home/$(whoami)/map_and_route

source /opt/ros/humble/setup.bash 
source /home/$(whoami)/GD-Autoware/install/setup.bash 

if [ -z "$PID" ]; then
  echo "No ros2 bag record process found."
else
  echo "Stopping ros2 bag record process with PID: $PID"
  # Send SIGINT (interrupt signal) to gracefully stop the process
  kill -INT $PID

  # Optionally, wait for the process to terminate and confirm
  sleep 1  # Wait for 1 second to allow the process to finish gracefully
  if ps -p $PID > /dev/null; then
    echo "Waiting for ros2 bag record process to stop..."
    sleep 5  # Wait for 5 seconds for the process to stop forcefully if needed
    if ps -p $PID > /dev/null; then
      echo "Forcefully stopping ros2 bag record process..."
      kill -KILL $PID  # Send SIGKILL if the process is still running
      sleep 1
    fi
  fi

  # Check if the process has terminated
  if ! ps -p $PID > /dev/null; then
    echo "ros2 bag record process stopped successfully."
    sleep 4
      BAG_NAME=$(cat /tmp/active_bag_name.txt)
      # Run the ROS 2 node and check if it was successful
      echo $BAG_DIR/$BAG_NAME/map_generator_config.yaml
      if ros2 run gd_route_map_generator_from_rosbag main $BAG_DIR/$BAG_NAME/map_generator_config.yaml; then
          echo "Command executed successfully."
          cp $BAG_DIR/$BAG_NAME/map_generator_config.yaml $TARGET_DIR/$BAG_NAME/map_generator_config.yaml
          echo "YAML file that contains params for creating the map and route copied to the target folder."
      else
          echo "Command failed."
          exit 1
      fi
  else
    echo "Failed to stop ros2 bag record process."
  fi
fi
