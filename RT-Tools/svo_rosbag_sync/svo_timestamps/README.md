# svo_timestamps
This tool generates a file containing the timestamps for each frame of the input SVO2 file, for usage with the **svo_rosbag_sync** package.

This tool was made because the ZED C++ API does expose the timestamps of SVO2 recordings, but only the current frame is known while replaying an SVO2 recording with the zed-ros2-wrapper. 

## USAGE
### Building
Building the tool from source shouldn't take anything special, for example you can follow these steps:
1. In the terminal, navigate to the *svo_timestamps* folder
2. Run the following commands:
   
```
mkdir build
cd build
cmake ..
make
```
### Running
Run the *SVO_Timestamps* executable with the path to the SVO2 file as its argument.
It should then automatically generate a timestamps file in the folder that contains the SVO2 file.

For example, running ```./SVO_Timestamps ~/test_recording/zed_recording.svo2``` will generate a new file ```~/test_recording/zed_recording.svo2.timestamps```