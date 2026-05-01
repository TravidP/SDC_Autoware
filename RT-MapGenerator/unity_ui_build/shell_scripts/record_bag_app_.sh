#!/bin/bash

source /opt/ros/humble/setup.bash 
source ~/GD-Autoware/install/setup.bash

# Define the default directory where the bag will initially be stored
BAG_DIR=/home/$USERNAME/my_recorded_bag
mkdir -p $BAG_DIR

# Define target directory for map and route files save
TARGET_DIR=/home/$USERNAME/map_and_route

# BAG_NAME=default1
# Define a default bag name if no argument is provided
timestamp=$(date +"%Y_%m_%d-%H_%M_%S")
DEFAULT_BAG_NAME=noname_rosbag_${timestamp}

# Check if a bag name argument is provided
if [ $# -eq 1 ]; then
    BAG_NAME=$1
else
    BAG_NAME=$DEFAULT_BAG_NAME
fi

echo "$BAG_NAME" > /tmp/active_bag_name.txt

# Specify the topics you want to record
TOPICS="
/fixposition/navsatfix
/fixposition/odometry
/tf
/tf_static
/fixposition/fpa/llh
/fixposition/fpa/llh_trans
/ecef_to_base_link/odometry
/fixposition/fpa/odometry
/localization/kinematic_state
/map/map_projector_info
/rosout"

# Record the bag with specified output directory and name
ros2 bag record $TOPICS -o $BAG_DIR/$BAG_NAME

WHEEL_BASE=$(ros2 param get /planning/mission_planning/mission_planner wheel_base | sed 's/^[^:]*:[ ]*//')
FRONT_OVERHANG=$(ros2 param get /planning/mission_planning/mission_planner front_overhang | sed 's/^[^:]*:[ ]*//')

# Create YAML file name to create map and route
map_generator_yaml_file=$BAG_DIR/$BAG_NAME/"map_generator_config.yaml"
cat <<EOL > $map_generator_yaml_file
map_generator_params:
    rosbag_file: $BAG_DIR/${BAG_NAME}/${BAG_NAME}_0.db3 
    output_folder: $TARGET_DIR/$BAG_NAME 
    speed_limit: 10.0
    lane_width: 3.0
    max_nodes_in_way: 15
    averaging_window_size: 5
    loop: false
    wheel_base: ${WHEEL_BASE}
    front_overhang: ${FRONT_OVERHANG}
EOL
echo "YAML file created: $map_generator_yaml_file"

