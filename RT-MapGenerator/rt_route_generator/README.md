# rt_route_generator

The **RouteGenerator** ROS2 node allows recording data into rosbags and generating maps/routes for autonomous vehicles for the path driven during recording. It integrates with Autoware vehicle info utilities to get information about the specific vehicle.

## Features

- Record sensor topics into rosbags.
- Generate lanelet maps with routes from recorded rosbags.

## Services

### `/record` (`rt_route_generator_msgs/srv/Record`)
- **Request**
  - `record` (bool): Start/stop recording
  - `rec_name` (string, optional): Bag name (defaults to current date-time)
- **Response**
  - `success` (bool)
  - `message` (string)

### `/create_map` (`rt_route_generator_msgs/srv/CreateMap`)
- **Request**
  - `create_map` (bool): Trigger map creation from the last recorded bag
- **Response**
  - `success` (bool)
  - `message` (string)

## Parameters

| Name                    | Type           | Description                                 |
|-------------------------|----------------|---------------------------------------------|
| `bag_directory`         | string         | Directory to store recorded rosbags         |
| `map_directory`         | string         | Directory to store generated maps           |
| `topic_names`           | string[]       | List of topics to record                    |
| `speed_limit`           | double         | Max speed for generated map                 |
| `lane_width`            | double         | Lane width for map generation               |
| `max_nodes_in_way`      | int            | Max nodes per route segment                 |
| `averaging_window_size` | int            | Window size for GPS smoothing               |
| `loop`                  | bool           | Whether generated routes should loop        |

