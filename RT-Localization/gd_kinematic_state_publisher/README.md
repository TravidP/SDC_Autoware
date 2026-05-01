# gd_kinematic_state_publisher

## Overview

`gd_kinematic_state_publisher` is a ROS 2 node that publishes a `nav_msgs/msg/Odometry` message representing the kinematic state of a robot in a target frame, using TF transforms and velocity information from an external odometry source.

The node:
- Looks up the pose of `target_frame` with respect to `source_frame` using TF
- Subscribes to an odometry topic providing linear and angular velocities at a fixed point (`FP_POI`)
- Transforms these velocities into the `base_link` (or configured target frame)
- Publishes a combined odometry message

## Node

### `GDKinematicStatePublisher`

## Published Topics

| Topic | Type | Description |
|------|------|-------------|
| `kinematic_state_topic` | `nav_msgs/msg/Odometry` | Odometry message containing pose from TF and transformed velocities |

## Subscribed Topics

| Topic | Type | Description |
|------|------|-------------|
| `fixpostion_odometry_topic` | `nav_msgs/msg/Odometry` | Odometry providing linear and angular velocity at `FP_POI` |

## Parameters

| Name | Type | Description |
|-----|------|-------------|
| `rate` | `int` | Publishing rate in Hz |
| `kinematic_state_topic` | `string` | Output odometry topic |
| `fixpostion_odometry_topic` | `string` | Input odometry topic |
| `source_frame` | `string` | Source frame for TF lookup (e.g. `map`) |
| `target_frame` | `string` | Target frame for TF lookup (e.g. `base_link`) |
| `tf_timeout` | `int` | TF lookup timeout in seconds |

## TF Requirements

The following TF transforms must be available:
- `source_frame → target_frame`
- `FP_POI → target_frame`

If the transform from `FP_POI` to `target_frame` is not available at startup, the node will terminate.

## TODO
- [ ] Include localization initialization state
- [ ] Add covariance