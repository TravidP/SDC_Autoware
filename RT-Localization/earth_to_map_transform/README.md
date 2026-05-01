# earth_to_map_transform

A ROS 2 package that establishes the transformation between Earth-Centered, Earth-Fixed (ECEF) coordinates and the local map frame using geographic map projector information.

## Overview

This package provides a node that subscribes to map projection information and publishes a static transform from the ECEF frame to the map frame. It bridges the gap between global geographic coordinates (latitude, longitude, altitude) and the local map coordinate system used in autonomous driving applications.

## Features

- Converts WGS84 geographic coordinates to ECEF coordinates
- Listens to TF transforms to obtain ECEF to ENU0 rotation
- Publishes static transform from `FP_ECEF` to `map` frame
- Reliable subscription with transient local durability for map projector info

## Usage

### Launch the Node

```bash
ros2 launch earth_to_map_transform earth2map_transform.launch.xml
```

### Launch Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `lookup_timeout` | double | 5.0 | Timeout duration (seconds) for TF transform lookup |

Example with custom timeout:
```bash
ros2 launch earth_to_map_transform earth2map_transform.launch.xml lookup_timeout:=10.0
```

### Subscribed Topics

| Topic | Type | Description |
|-------|------|-------------|
| `/map/map_projector_info` | `autoware_map_msgs/msg/MapProjectorInfo` | Map projection origin information (latitude, longitude, altitude) |

**QoS Settings:**
- Reliability: Reliable
- Durability: Transient Local
- History: Keep Last (depth: 1)

### Published Transforms

| Parent Frame | Child Frame | Description |
|--------------|-------------|-------------|
| `FP_ECEF` | `map` | Static transform from ECEF to map frame |

### Required Transforms

The node requires the following transform to be available:
- `FP_ECEF` → `FP_ENU0`: Rotation from ECEF to local East-North-Up frame

## How It Works

1. The node subscribes to map projector information containing the map origin in geographic coordinates (WGS84)
2. When map projector info is received:
   - Converts the map origin from WGS84 (lat/lon/alt) to ECEF coordinates using GeographicLib
   - Looks up the rotation transform from `FP_ECEF` to `FP_ENU0`
   - Publishes a static transform from `FP_ECEF` to `map` with:
     - Translation: ECEF coordinates of the map origin
     - Rotation: Same rotation as ECEF to ENU0

## Coordinate Frames

- **FP_ECEF**: Earth-Centered, Earth-Fixed frame (origin at Earth's center)
- **FP_ENU0**: Local East-North-Up frame at a reference point
- **map**: Local map frame used by the autonomous driving system

## Troubleshooting

### Transform Exception Error

If you see errors like `Transform exception: ...`, check that:
- The `FP_ECEF` to `FP_ENU0` transform is being published
- The transform is available before the map projector info arrives
- The `lookup_timeout` parameter is sufficient for your system

### No Map Projector Info

If the transform is not published:
- Verify that `/map/map_projector_info` topic is being published
- Check the topic QoS settings match (Reliable, Transient Local)
- Use `ros2 topic echo /map/map_projector_info` to verify message receipt
