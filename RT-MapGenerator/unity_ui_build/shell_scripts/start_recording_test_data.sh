#!/bin/bash

source /opt/ros/humble/setup.bash 
source ~/GD-Autoware/install/setup.bash

BAG_DIR=/home/$USERNAME/recorded_test_data
mkdir -p $BAG_DIR

ROUTE_NAME=$1
BAG_NAME=${ROUTE_NAME}_$(date +%F_%T)

EXCLUDED_TOPICS="/zed/(.*)|/nebula/(.*)|/gd_point_cloud/(.*)|/perception/obstacle_segmentation/pointcloud|/yolo/dbg_image"

# Record the bag with specified output directory and name
ros2 bag record -a -x $EXCLUDED_TOPICS -o $BAG_DIR/$BAG_NAME --compression-mode file --compression-format zstd

