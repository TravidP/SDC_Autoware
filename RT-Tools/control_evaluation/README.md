# Control Evaluation
This folder contains two shell scripts, one for data collection and one for offline data replaying surrounding vehicle control as well as a small plotjuggler layout format.

## control_rec.sh
- Creates a directory on Desktop named "control_perf+month/day/time"
- Copies CONFIG_FILE (currently pointing at the mpc autoware params) to the created directory
- Launches a tmux session with two windows. One with control_performance_analysis node from autoware and one that records a rosbag with the topics provided in topic_list and saves it under bag/ in the created directory (there is a rudimentary check if the requested topics are all present before the recording begins).

## control_playback.sh
- Starts tmux session with two panes.
- First pane starts the rosbag found in the provided_path/bag/bag_0.db3 in pause mode (press spacebar to resume). path is the first argument.
- Second pane launches plotjuggler with mpc_layout.xml in the script's directory, buffer size 100 and ROS2_STREAMER.

