# RT-Sensing

ROS2 launch and description repo for sensor drivers and their pipelines.
Currently using xsens, zed_camera and [ARS540 radar](https://github.com/GreenDinoBV/GD-SensorLaunch#ARS540-Radar-Setup).

## gd_*vehicle*_sensor_launch package

config  
Contains parameter .yaml files for the different sensors

---
launch  
Contains standalone launch files for all the sensors as well as a launch file that launches all the sensor at the same time.

### Dependencies

- fixposition_driver (https://github.com/fixposition/fixposition_driver.git)
- zed_ros2_wrapper (https://github.com/stereolabs/zed-ros2-wrapper.git)

## gd_*vehicle*_sensor_description package
The main purpose of this package is to describe the sensor frame IDs, calibration parameters of all sensors, and their links with urdf files.

---
config

- sensors_calibration.yaml:  
Defines the mounting positions and orientations of sensors (child frame) with base_link as the parent frame. At Autoware, base_link is on projection of the rear-axle center onto the ground surface.

---
urdf

- sensors.xacro: Links the sensor_kit main frame (sensor_kit_base_link) to base_link. Also sensors which were calibrated directly to base_link, are added here.

## ARS540 Radar Setup

### Initial check
To make sure the computer is receiving data from the radar, you can use Wireshark.
In Wireshark you should be able to see the radar messages coming in constantly, with source 10.13.1.113 and destination 224.0.2.2.

### VLAN 19
The radar sends and receives messages on VLAN 19. 
Configuring this VLAN in Ubuntu can be done by running `nm-connection-editor`, adding a new VLAN with the main network interface as its parent (e.g. eno1) and VLAN id 19.
Set the VLAN interface name to `vlan.19`. 
In the IPv4 Settings tab set the method to 'Manual' and add an address 10.13.1.166 with netmask 24.

### Setting radar parameters

#### NvM
The radar has its configuration stored in 'non-volatile memory', which only needs to be configured once.
Continental warns that "excessive reconfiguration of the sensor may lead to a non-functional sensor, as the NvM has a limited number of write operations."

#### Nebula ARS548 support
To set the radar parameters, you can use the Nebula, which is automatically included as an external package when installing Autoware from source.
Autoware has support for the Continental ARS548 radar, but the ARS548 driver also works for the ARS540 radar that we have.
The differences between these two radar models are shown below: [ARS540 vs ARS548](https://github.com/GreenDinoBV/GD-SensorLaunch#ARS540-vs-ARS548).

The Nebula ARS548 driver can be launched by running:

`ros2 launch nebula_ros continental_launch_all_hw.xml sensor_ip:=10.13.1.113`

#### View current parameters
While the Nebula driver is running, the current radar settings are sent to the `/diagnostics` topic.
(Note: if the Nebula driver was launched with the ars540.launch.xml file from this repository, the radar settings can be found by echoing the `/nebula/diagnostics` topic instead.)

#### Changing the parameters
The definitions of the parameters can be found in section 5.1.1 of the technical documentation of the radar (ARS540_TPS.pdf provided by Continental to robotTUNER).

Examples of setting the parameters through Nebula:

`ros2 service call /set_sensor_mounting continental_srvs/srv/ContinentalArs548SetSensorMounting "{autoconfigure_extrinsics: False, longitudinal: 2.805, lateral: 0, vertical: 0.68, yaw: 0, pitch: -0.0418, plug_orientation: 1}"`

`ros2 service call /set_vehicle_parameters continental_srvs/srv/ContinentalArs548SetVehicleParameters "{vehicle_length: 12.170, vehicle_width: 2.550, vehicle_height: 3.095, vehicle_wheelbase: 6.035}"`

### Odometry input

According to section 5.1.4 of the technical documentation, the only required dynamic parameters are yaw rate, vehicle speed and driving direction.
These parameters are obtained from the `velocity` field from the `/fixposition/fpa/odomsh` topic and provided to the Nebula node by using the `relay_field` tool. 
Continental recommends providing the other parameters (acceleration and steering wheel angle) as well, this is currently on the TODO list of this document.

To verify that the radar is receiving the odometry data correctly, you can look at the diagnostics messages published by the radar.
In the current configuration, they are published to the `/nebula/diagnostics` topic.
The fields `longitudinal_velocity_status`, `yaw_rate_status` and `driving_direction_status` should all have value `0:VDY_OK` (where VDY stands for Vehicle Dynamics).

### ARS540 vs ARS548

While the ARS540 and ARS548 aren't exactly the same, the ARS548 driver has worked for the ARS540 radar so far without issues.

Comparing the message contents in the ARS548 driver source code with the ARS540 Ethernet Interface (ARS540_IO.pdf provided by Continental to robotTUNER), the only difference seems to be in the Status message.
The Status messages of the ARS548 contain three additional fields when compared to the Status messages of the ARS540:
- `voltage_status`
- `temperature_status`
- `blockage_status`

The absence of these three fields from the ARS540 messages does not seem to interfere with the correct working of the driver.

## TODO
- [ ] Provide acceleration and steering wheel angle to the radar (recommended by Continental)
