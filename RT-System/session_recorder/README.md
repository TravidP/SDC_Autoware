# Session Recorder

`session_recorder` is a ROS 2 package that provides a **smart rosbag recorder node**. It allows you to automatically record selected topics, whole namespaces, or all topics under certain conditions. It also monitors disk space, supports auto-starting, and exposes services to control recording at runtime.

---

## Features
- Record:
  - A list of specific topics.
  - All topics in given namespaces.
  - Combination of both.
- Smart topic expansion (namespace + topic joining).
- Auto-start recording on node startup (configurable).
- Disk space monitoring with threshold-based stop.
- Services to start/stop recording on demand.
- Publishes periodic status messages.
- Customizable bag name prefix and output directory.

---

### Parameters
| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `topics` | list[string] | `[]` | Specific topics to record. |
| `namespaces` | list[string] | `[]` | Namespaces to include (all topics under these namespaces). |
| `record_all_namespace_topics` | bool | `true` | If `true`, record all topics found in given namespaces. If `false`, only expand explicitly listed topics with namespaces. |
| `output_directory` | string | `./bags` | Directory to store bag files. |
| `bag_name_prefix` | string | `recording` | Prefix for bag file names. Timestamp is appended automatically. |
| `auto_start` | bool | `false` | Whether to automatically start recording when the node launches. |
| `space_threshold_gb` | double | `1.0` | Minimum free space (in GB) required to continue recording. |
| `check_interval_seconds` | double | `10.0` | Disk space monitoring interval. |

### Topics
- `~/status` (`std_msgs/String`) → periodic status messages (e.g., recording state, bag size, free space).

### Services
- `~/start_recording` (`std_srvs/Trigger`) → start recording manually.
- `~/stop_recording` (`std_srvs/Trigger`) → stop recording manually.

### Status Messages
The status publisher reports every 5 seconds (default) with information such as:
- Current state (`RECORDING` / `IDLE`).
- Bag file name and size.
- Available disk space.


## Development Notes
- `getTopicsToRecord()` is responsible for resolving which topics are actually recorded based on parameters.
- Bags are written using `rosbag2_cpp::Writer` with SQLite3 backend.
- The node uses a monitoring thread to periodically check disk space.

---


