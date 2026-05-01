# ecef_to_geopose_transformation

A ROS 2 package that transforms odometry data from Earth-Centered, Earth-Fixed (ECEF) coordinates to geographic pose (GeoPose) with East-North-Up (ENU) orientation.

## Overview

This package provides a node that converts ECEF odometry messages into geographic poses with latitude, longitude, altitude, and ENU-oriented quaternions. It's designed for real-time localization systems that need to bridge between global ECEF coordinates and geographic representations suitable for navigation and visualization.

## Features

- Converts ECEF position (X, Y, Z) to geographic coordinates (latitude, longitude, altitude)
- Transforms ECEF orientation to ENU (East-North-Up) frame orientation
- Maintains original timestamps for minimal latency
- Publishes processing time diagnostics
- Configurable input/output topics via parameters

## Usage

### Launch the Node

```bash
ros2 launch ecef_to_geopose_transformation ecef2geopose.launch.xml
```

### Configuration

The node is configured via a YAML parameter file located at `config/ecef2geopose.param.yaml`:

```yaml
/**:
  ros__parameters:
    input_topic: /fixposition/odometry_ecef
    output_topic: /localization/geopose
```

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `input_topic` | string | Yes | Topic name for incoming ECEF odometry messages |
| `output_topic` | string | Yes | Topic name for outgoing GeoPose messages |

### Subscribed Topics

| Topic | Type | QoS | Description |
|-------|------|-----|-------------|
| `<input_topic>` | `nav_msgs/msg/Odometry` | Best Effort, Volatile | ECEF odometry from positioning system |

**Default:** `/fixposition/odometry_ecef`

**QoS Settings:**
- Reliability: Best Effort
- Durability: Volatile
- History: Keep Last (depth: 1)

### Published Topics

| Topic | Type | Description |
|-------|------|-------------|
| `<output_topic>` | `geographic_msgs/msg/GeoPoseWithCovarianceStamped` | Geographic pose with ENU orientation |
| `~/proc_time` | `std_msgs/msg/Float64` | Processing time in milliseconds |

**Default output:** `/localization/geopose`

## How It Works

The transformation process follows these steps:

1. **ECEF to Geographic Conversion**
   - Receives ECEF coordinates (X, Y, Z) from odometry message
   - Uses GeographicLib to compute latitude, longitude, and altitude (WGS84)

2. **Orientation Transformation**
   - Extracts quaternion from ECEF frame odometry
   - Computes ECEF to ENU rotation matrix based on geographic position
   - Applies rotation: `R_POI_ENU = R_ECEF_ENU × R_POI_ECEF`
   - Converts resulting rotation back to quaternion

3. **Message Publication**
   - Publishes geographic pose with original timestamp
   - Publishes processing time for diagnostics

## Coordinate Frames

- **ECEF (Earth-Centered, Earth-Fixed)**: Origin at Earth's center, X-axis through prime meridian, Z-axis through north pole
- **POI (Point of Interest)**: Local frame attached to the vehicle/sensor
- **ENU (East-North-Up)**: Local tangent plane frame with East-North-Up orientation at the geographic position

## Message Format

### Input: nav_msgs/msg/Odometry
```
header:
  stamp: <timestamp>
  frame_id: "ecef"
pose:
  pose:
    position: {x, y, z}      # ECEF coordinates (meters)
    orientation: {x, y, z, w} # Quaternion in ECEF frame
```

### Output: geographic_msgs/msg/GeoPoseWithCovarianceStamped
```
header:
  stamp: <same_timestamp>
pose:
  pose:
    position:
      latitude: <degrees>
      longitude: <degrees>
      altitude: <meters>
    orientation: {x, y, z, w}  # Quaternion in ENU frame
```

## Performance

The node is optimized for low-latency operation:
- Maintains original message timestamps
- Minimal computational overhead
- Processing time typically < 1ms (published on `~/proc_time`)

## Integration Example

Typical integration with a positioning system:

```
[Fixposition Sensor] → /fixposition/odometry_ecef 
                              ↓
                    [ecef2geopose node]
                              ↓
                    /localization/geopose → [Navigation Stack]
```

## Troubleshooting

### No Output Messages

If the node is running but not publishing:
- Verify input topic is publishing: `ros2 topic echo <input_topic>`
- Check QoS compatibility (node uses Best Effort)
- Confirm parameter file is loaded correctly

### Invalid Coordinates

If output coordinates seem incorrect:
- Verify input ECEF coordinates are valid (typically millions of meters)
- Check that orientation quaternions are normalized
- Ensure GeographicLib is properly installed

### High Processing Time

If `~/proc_time` shows unusually high values:
- Check CPU load and system resources
- Verify message rate is appropriate for your system
- Consider the rate of incoming odometry messages

## Performance Monitoring

Monitor processing time in real-time:
```bash
ros2 topic echo /ecef2geopose/proc_time
```

Typical processing times should be under 1-2 milliseconds on modern hardware.

## Advanced Configuration

### Custom Topic Names

Modify the parameter file or override at launch:

```bash
ros2 run ecef_to_geopose_transformation ecef2geopose \
  --ros-args \
  -p input_topic:=/my/custom/ecef/topic \
  -p output_topic:=/my/custom/geopose/topic
```

## Mathematical Background

The ECEF to ENU rotation matrix is computed as:

```
R_ECEF_ENU = [-sin(lon)           cos(lon)            0        ]
              [-sin(lat)*cos(lon)  -sin(lat)*sin(lon)  cos(lat) ]
              [cos(lat)*cos(lon)   cos(lat)*sin(lon)   sin(lat) ]
```

Where lat and lon are the latitude and longitude of the position in radians.

## References

- [GeographicLib Documentation](https://geographiclib.sourceforge.io/)
- [ECEF Coordinate System](https://en.wikipedia.org/wiki/Earth-centered,_Earth-fixed_coordinate_system)
- [ENU Coordinate System](https://en.wikipedia.org/wiki/Local_tangent_plane_coordinates)
