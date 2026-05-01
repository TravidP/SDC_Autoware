timestamp=$(date +%F_%T)
directory=~/sensor_input_recordings/$timestamp
mkdir -p $directory

# start zed recording
ros2 service call /zed/zed_node/start_svo_rec zed_interfaces/srv/StartSvoRec "{bitrate: 60000, compression_mode: 1, target_framerate: 30, input_transcode: false, svo_filename: $directory/zed_recording.svo}"

# start ros bag recording
gnome-terminal -- ros2 bag record -a -x "/zed/(.*)|/gd_point_cloud/(.*)|/perception/obstacle_segmentation/pointcloud|/yolo/dbg_image" -o $directory/rosbag --compression-mode file --compression-format zstd
pid=$(ps ax | grep 'bag record' | head -1 | awk '{ print $1 }')

echo "Recording started, press any key to stop recording..."

# -s: Do not echo input coming from a terminal
# -n 1: Read one character
read -s -n 1

# stop zed recording
ros2 service call /zed/zed_node/stop_svo_rec std_srvs/srv/Trigger "{}"

# stop ros bag recording
kill $pid 

echo "Recording stopped; stored in directory with name '$directory'"
