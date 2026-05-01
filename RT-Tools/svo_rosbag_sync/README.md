# svo_rosbag_sync
This package replays specific topics from a ROS2 bag, synchronized with a ZED recording in SVO2 format.
The purpose of this package is to replay all external input (initially from the Xsens, ZED camera and radar) so it can be replayed for tests without needing access to an actual vehicle or actual sensors.

For synchronization to work, an additional 'timestamps' file is needed that contains the timestamp for each frame of the SVO2 recording.
This timestamps file can be generated with the **svo_timestamps** tool that is also included in this folder. Instructions for its usage can be found in its accompanying readme file.

Only specific topics from the ROS2 bag are replayed, defined (hardcoded) in the *SvoRosbagSyncNode* class.
To add a new topic, first make sure its type is present in *CMakeLists.txt* and *package.xml*.
Then add the topic to both *SvoRosbagSyncNode.hpp* and *SvoRosbagSyncNode.cpp*, in a similar way to the topics that are already there. 

This package also replays TF messages, but only for specific frames. These frames are defined in *TfTopicSync.cpp*.

## USAGE
0. (Optional) Record a ZED SVO2 file and corresponding ROS2 bag on a system with the zed-ros2-wrapper running and the other sensors sending their output to ROS2.
   - You can use the provided **record_sensor_input.sh** shell script for this, which will automatically save a recording to a new folder ```~/sensor_input_recordings/<current_timestamp>```.
   - When using this shell script, make sure to end the recording by pressing a key in the terminal where it was started, because e.g. simply killing the process won't send a 'stop svo recording' call to ZED, and it will keep recording.
1. Make sure you have a ZED SVO2 file and a corresponding ROS2 bag together in the same folder using the following file structure:
   - \<name of recording>
     - rosbag
         - rosbag_0.db3
         - metadata.yaml
     - zed_recording.svo2
2. Generate a timestamps file with the **svo_timestamps** tool (included in this folder, see its readme for instructions).
   The file structure should now look like this:
    - \<name of recording>
      - rosbag
        - rosbag_0.db3
        - metadata.yaml
      - zed_recording.svo2
      - zed_recording.svo2.timestamps
3. Run the following command:

   ```ros2 launch svo_rosbag_sync gd_replay.launch.xml recording_directory:=<recording_folder_path>```
4. (Optional) Start autopilot systems, preferably with the same map that was used while making the recording
   - Note: starting the autopilot too soon can cause the autopilot to not work correctly. To prevent this from happening, do not start the autopilot before the ```synchronizing; current offset: ...``` messages start appearing in the svo_rosbag_sync output.