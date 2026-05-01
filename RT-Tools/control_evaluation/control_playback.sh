#!/bin/bash

#Starts a bag created be control_rec.sh in paused mode (use spacebar to resume) along with plotjuggler
#using the configuration provided in mpc_layout.xml 

SESSION_NAME="control_eval"
LAYOUT_PATH="$(pwd)/mpc_layout.xml"
RESULT_FOLDER="$1"

tmux new-session -d -s ${SESSION_NAME}
tmux send-keys -t ${SESSION_NAME} "ros2 bag play ${RESULT_FOLDER}/bag/bag_0.db3 --start-paused" C-m

tmux split-window -v -t ${SESSION_NAME}
tmux send-keys -t ${SESSION_NAME} "ros2 run plotjuggler plotjuggler --layout ${LAYOUT_PATH} --buffer_size 100 --start_streamer "ROS2_Streamer" "  C-m

tmux attach -t ${SESSION_NAME}
