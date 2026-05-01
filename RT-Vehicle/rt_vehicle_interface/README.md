# RT Vehicle Interface

The **RT Vehicle Interface** is a ROS 2 node that translates high-level Autoware vehicle control commands into low-level commands understood by a specific vehicle controller (LLC).
It provides a **compile-time, vehicle-specific adaptation layer** using C++ templates (CRTP) instead of runtime polymorphism.

Currently supported vehicles:
- **Twizy**
- **Citaro**

The design allows new vehicle types to be added with zero runtime overhead and no virtual dispatch.

---

## Architecture Overview

<pre>
Autoware
   |
   |  /control/command/*
   v
RT Vehicle Interface (GDInterfaceROS2<T>)
   |
   |  vehicle-specific conversions
   v
Low-Level Controller (LLC)
</pre>

---

### Key Components

- **`GDInterfaceROS2<T>`**
  - ROS 2 node handling subscriptions and publications
  - Templated on the vehicle type
  - Publishes converted commands to the LLC
  - Periodically publishes a heartbeat

- **`RosVehicleAdapter<T>`**
  - CRTP base class
  - Defines the required interface for vehicle adapters
  - Ensures compile-time enforcement of conversion functions

- **Vehicle implementations**
  - `Twizy`
  - `Citaro`
  - Provide vehicle-specific steering, gear, indicator, and speed conversions

---

## Topics

### Subscribed Topics (from Autoware)

| Topic | Type |
|------|------|
| `/control/command/control_cmd` | `autoware_control_msgs/msg/Control` |
| `/control/command/gear_cmd` | `autoware_vehicle_msgs/msg/GearCommand` |
| `/control/command/turn_indicators_cmd` | `autoware_vehicle_msgs/msg/TurnIndicatorsCommand` |

---

### Published Topics (to LLC)

| Topic | Type | Description |
|------|------|------------|
| `/vehicle_controller/target_speed_acc` | `std_msgs/msg/Float32MultiArray` | `[speed, acceleration]` |
| `/vehicle_controller/target_steering_angle` | `std_msgs/msg/Int64` | Steering wheel angle |
| `/vehicle_controller/indicator_command` | `std_msgs/msg/UInt8` | Turn indicator command |
| `/vehicle_controller/gear_command` | `std_msgs/msg/UInt8` | Gear command |

---

### Heartbeat

| Topic | Type |
|------|------|
| `vehicle/interface_heartbeat` | `tier4_external_api_msgs/msg/Heartbeat` |

- Published every **300 ms**
- Used to monitor node liveness

---

## QoS Configuration

- **Best effort**
- **Keep last (depth = 1)**
- **Volatile durability**
- **Deadline: 50 ms** (for speed and steering publishers)

Deadline misses are logged with a warning.

---

## Supported Vehicles

### Twizy
- Steering conversion based on tire-to-wheel ratio
- Autoware gear and indicator mappings specific to Twizy hardware

### Citaro
- Different steering ratio
- Same Autoware command mapping logic with vehicle-specific parameters

---


