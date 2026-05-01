#!/bin/bash

#Script for mpc trajectory evaluation.
# - Makes BAG_DIR and copies the CONFIG_FILE to BAG_DIR
# - Launches control_performance_analysis node.
# - Checks that all the topics in topic_list exist (somewhat).
# - Starts recording provided topics.

formatted_date=$(date +"%m_%d_%H:%M:%S")

SESSION_NAME="control_perf_rec"
BAG_DIR="/home/$(whoami)/Desktop/control_perf${formatted_date}"
CONFIG_FILE="/home/$(whoami)/autoware/src/launcher/autoware_launch/autoware_launch/config/control/trajectory_follower/lateral/mpc.param.yaml"

topic_list=(
	"/planning/scenario_planning/trajectory"
 	"/control/command/control_cmd"
	"/vehicle/status/steering_status"
	"/localization/kinematic_state"
	"/tf"
	"/tf_static"
	"/control_performance/driving_status"
	"/control_performance/performance_vars"
)


mkdir ${BAG_DIR}
cp ${CONFIG_FILE} ${BAG_DIR}/


tmux new-session -d -s ${SESSION_NAME}
tmux send-keys -t ${SESSION_NAME} "ros2 launch control_performance_analysis control_performance_analysis.launch.xml" C-m

sleep 3
for topic in "${topic_list[@]}"; do
	if [ -z "$(ros2 topic list | grep ${topic})" ]; then
		echo "Topic ${topic} not present. Exiting..."
		tmux kill-session -t ${SESSION_NAME}
		exit 1
	fi
done

tmux split-window -v -t ${SESSION_NAME}
tmux send-keys -t ${SESSION_NAME} "ros2 bag record -o ${BAG_DIR}/bag ${topic_list[*]}" C-m

tmux attach -t ${SESSION_NAME}


