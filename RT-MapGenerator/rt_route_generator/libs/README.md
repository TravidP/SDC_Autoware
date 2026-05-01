# Map Generator

This package generates **Lanelet2-compatible maps** from a sequence of GPS coordinates. It produces both a Lanelet2 `.osm` map and a corresponding map projector configuration file, suitable for use in autonomous driving and simulation pipelines.

## Overview

Given an ordered list of latitude/longitude coordinates (typically from GNSS data), the map generator:
- Computes a centerline direction
- Offsets left and right lane boundaries using a configurable lane width
- Splits long boundaries into multiple OSM ways
- Creates Lanelet2 relations with speed limits
- Applies optional smoothing to lane boundaries

## Outputs

- `lanelet2_map.osm`  
  Lanelet2 map containing nodes, ways, and lanelet relations.

- `map_projector_info.yaml`  
  Projection configuration using a Transverse Mercator projection with the map origin set to the first GPS coordinate.

## Dependencies

- C++17
- Eigen
- Boost.Geometry
- Lanelet2 (`lanelet2_projection`)
- Standard C++ STL


# Rosbag Handler

This module provides a lightweight utility for **reading ROS 2 bag files** and extracting a selected set of messages required for map generation and localization analysis. It deserializes messages directly from a rosbag and returns them in a structured result container.

## Overview

`RosbagHandler` iterates through a ROS 2 bag file and collects messages from specific topics related to GNSS, odometry, localization, and map projection. GNSS positions are filtered by distance to reduce redundant samples.

## Supported Topics

The following topics are read if present in the rosbag:

| Topic | Message Type | Description |
|------|-------------|-------------|
| `/fixposition/fpa/llh` | `fixposition_driver_msgs::msg::FpaLlh` | GNSS latitude/longitude/height data |
| `/fixposition/fpa/odometry` | `fixposition_driver_msgs::msg::FpaOdometry` | Fixposition odometry output |
| `/ecef_to_base_link/odometry` | `nav_msgs::msg::Odometry` | ECEF to `base_link` transform |
| `/localization/kinematic_state` | `nav_msgs::msg::Odometry` | Vehicle kinematic state |
| `/map/map_projector_info` | `autoware_map_msgs::msg::MapProjectorInfo` | Map projection metadata |

All other topics are ignored.

## Distance Filtering

GNSS samples from `/fixposition/fpa/llh` are added only if the distance from the previously stored sample exceeds **0.1 meters**. Distance is computed using a Haversine-based function.

This reduces noise and redundant points when generating maps.

## Dependencies

- ROS 2
- rosbag2_cpp
- rclcpp
- nav_msgs
- fixposition_driver_msgs
- autoware_map_msgs

## Error Handling

- Exceptions during rosbag access are caught and reported to stderr
- The function returns a partially filled result if an error occurs


# Rosbag Recorder

This module provides a simple C++ interface for **starting and stopping ROS 2 bag recordings** by launching the `ros2 bag record` command in a separate process. It is designed for programmatic control of rosbag recording from non-ROS nodes or backend services.

## Overview

`RosbagRecorder` uses **Boost.Process** to spawn and manage a `ros2 bag record` process. It supports recording all topics or a selected subset of topics, custom output bag names, and ensures safe start/stop behavior through mutex protection.

The recorder automatically sources the ROS 2 environment before launching the recording command.

## Features

- Start and stop rosbag recording programmatically
- Record all topics or a selected list of topics
- Customizable output directory and bag name
- Process lifecycle management (PID tracking, termination)
- Thread-safe start/stop operations


## API
### startRecording

```
Result startRecording(
  const std::vector<std::string> & topics,
  const std::string & bag_name = ""
);
```

- If topics is empty, all topics are recorded (-a)
- If bag_name is empty, ROS 2 assigns a timestamp-based name
- Returns a Result indicating success or failure

### stopRecording

```Result stopRecording();```

Stops the active recording process if one is running.\
Result\
success_ ---> Indicates whether the operation succeeded\
msg_ ---> Informational or error message

## Implementation Details

Recording is started via:

```ros2 bag record [topics] -o <output_path>```

The ROS 2 Humble environment is sourced explicitly:

- source /opt/ros/humble/setup.bash
- Standard output and error streams are suppressed
- A short delay is used to verify successful process startup

## Dependencies

- ROS 2 (Humble)
- Boost.Process
- C++17
- Standard C++ STL

## Thread Safety

- startRecording() and stopRecording() are protected by a mutex
- Prevents multiple concurrent recording processes
## Notes and Limitations

- ROS distribution path is hardcoded (/opt/ros/humble)
- Recording process is terminated via terminate() (SIGTERM)
- Intended for Linux-based systems
- The default output directory is the user’s home directory if none is provided
## Intended Use Cases

- Programmatic rosbag control from GUI or backend applications
- Data collection pipelines
- On-demand recording during experiments or testing

# Route Creator

This module generates **vehicle route files** from recorded ROS 2 messages. It converts GNSS and odometry data into CSV files containing time-stamped vehicle poses, suitable for offline analysis, visualization, or replay in simulation and autonomous driving pipelines.

## Overview

`RouteCreator` supports two route generation modes:

1. **GNSS-based route generation**  
   Converts Fixposition LLH messages into a locally projected 2D trajectory with orientation inferred from consecutive points.

2. **Odometry-based route generation**  
   Writes vehicle poses directly from ECEF-to-`base_link` odometry messages.

All outputs are written in CSV format.

## Outputs

### `route_navsatfix.csv`

Generated from GNSS data (`FpaLlh`):

pose_x,pose_y,pose_z,orientation_x,orientation_y,orientation_z,orientation_w,timestamp


- Positions are projected into a local Transverse Mercator frame
- Orientation (yaw) is computed from successive trajectory points

### `vehicle_pose.csv`

Generated from odometry data (`nav_msgs::msg::Odometry`):

pose_x,pose_y,pose_z,orientation_x,orientation_y,orientation_z,orientation_w,timestamp


- Positions and orientations are written directly from the odometry messages

## Projection

- GNSS points are projected using a **Transverse Mercator projection**
- The projection origin is set to the **first GNSS sample**
- Only planar (x, y) motion is considered; z is set to zero for GNSS-based routes


## Implementation Details
- Orientation is computed as:\
yaw = atan2(dy, dx) between consecutive poses
- Orientation is stored as a quaternion
ROS time stamps are converted to seconds with nanosecond precision
- CSV files use fixed-point formatting for numerical stability
## Dependencies

- ROS 2
- tf2
- nav_msgs
- fixposition_driver_msgs
- autoware_lanelet2_extension

## Notes and Limitations

- GNSS-based route generation ignores altitude
- The last GNSS point is not written (orientation requires a forward point)
- Intended for offline processing
- No filtering or smoothing is applied to the trajectory

## Intended Use Cases

- Route generation for simulation
- Offline trajectory visualization
- Preprocessing recorded data for map or route-based planning
