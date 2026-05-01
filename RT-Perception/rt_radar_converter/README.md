# robotTUNER Radar Converter

This package converts the data obtained from the radar (through Nebula) into data that can be used by Autoware.

The objects detected by the radar are currently published as [predicted objects](https://github.com/autowarefoundation/autoware_msgs/blob/main/autoware_perception_msgs/msg/PredictedObject.msg),
even though no actual prediction is done.
In the future this node should ideally publish either [detected objects](https://github.com/autowarefoundation/autoware_msgs/blob/main/autoware_perception_msgs/msg/DetectedObject.msg)
or [tracked objects](https://github.com/autowarefoundation/autoware_msgs/blob/main/autoware_perception_msgs/msg/TrackedObject.msg)
(because the radar has its own tracking functionality).
More processing on the objects, like filtering, prediction and possibly tracking, should be done by other nodes. 

Current functionality includes:
- Converting the position and orientation of the objects
- Loading the size of the objects (the radar only provides the horizontal size, object heights are hardcoded to 2 meters)
  - This node has a parameter `use_fixed_object_width`, which makes it ignore the object sizes and always gives them the width defined with the `fixed_object_width` parameter.
- Converting the 32 bit Continental object ID to an 128 bit Autoware object ID (UUID)
- Loading the existence probability of the objects
- Converting object classifications, using the following conversions:

| Continental classification | Autoware classification |
|---|---|
| Car | Car |
| Truck | Truck |
| Motorcycle | Motorcycle |
| Bicycle | Bicycle |
| Pedestrian | Pedestrian |
| Animal | Unknown |
| Hazard | Unknown |
| Unknown | Unknown |
| Overdrivable | Unknown |
| Underdrivable | Unknown |

## TODO
- Load additional features from the Continental objects into Autoware objects, most notably speed and acceleration
- Publish either detected objects or tracked objects, as discussed above