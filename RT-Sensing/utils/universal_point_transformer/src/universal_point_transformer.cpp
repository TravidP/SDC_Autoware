// universal_point_transformer.cpp
#include "universal_point_transformer/universal_point_transformer.hpp"

#include <memory>
#include <rclcpp_components/register_node_macro.hpp>
#include <string>

namespace universal_point_transformer
{
// using autoware::point_types;
UniversalPointTransformerComponent::UniversalPointTransformerComponent(
  const rclcpp::NodeOptions & options)
: Node("universal_point_transformer", options)
{
  // Declare parameters
  declare_parameter("input_topic", "points_raw");
  declare_parameter("output_topic", "points_xyzirc");

  // Get parameters
  std::string input_topic = get_parameter("input_topic").as_string();
  std::string output_topic = get_parameter("output_topic").as_string();

  // Set up subscription and publisher
  publisher_ =
    create_publisher<sensor_msgs::msg::PointCloud2>(output_topic, rclcpp::SensorDataQoS());

  subscription_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    input_topic, rclcpp::SensorDataQoS(),
    std::bind(
      &UniversalPointTransformerComponent::pointCloudCallback, this, std::placeholders::_1));

  RCLCPP_INFO(
    get_logger(), "UniversalPointTransformer initialized with input: %s, output: %s",
    input_topic.c_str(), output_topic.c_str());
}

void UniversalPointTransformerComponent::pointCloudCallback(
  const sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
  // Check if already in target format
  if (isCompatibleWithPointXYZIRC(*msg)) {
    RCLCPP_DEBUG(get_logger(), "Point cloud already in PointXYZIRC format, passing through");
    publisher_->publish(*msg);
    return;
  }

  // Transform to target format
  auto transformed_cloud = transform_to_xyzirc_compatible(*msg);
  publisher_->publish(transformed_cloud);
}

InputPointType UniversalPointTransformerComponent::detectInputPointType(
  const sensor_msgs::msg::PointCloud2 & cloud)
{
  // Check if it's XYZI format
  if (
    cloud.fields.size() == 4 && cloud.fields[0].name == "x" &&
    cloud.fields[0].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[1].name == "y" &&
    cloud.fields[1].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[2].name == "z" &&
    cloud.fields[2].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[3].name == "intensity" &&
    cloud.fields[3].datatype == sensor_msgs::msg::PointField::FLOAT32) {
    return InputPointType::XYZI;
  }

  // Check if it's XYZRGB format
  if (
    cloud.fields.size() == 4 && cloud.fields[0].name == "x" &&
    cloud.fields[0].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[1].name == "y" &&
    cloud.fields[1].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[2].name == "z" &&
    cloud.fields[2].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[3].name == "rgb" &&
    cloud.fields[3].datatype == sensor_msgs::msg::PointField::FLOAT32) {
    return InputPointType::XYZRGB;
  }

  // Check if it's XYZII format
  if (
    cloud.fields.size() == 5 && cloud.fields[0].name == "x" &&
    cloud.fields[0].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[1].name == "y" &&
    cloud.fields[1].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[2].name == "z" &&
    cloud.fields[2].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[3].name == "intensity" &&
    cloud.fields[3].datatype == sensor_msgs::msg::PointField::FLOAT32 &&
    cloud.fields[4].name == "index" &&
    cloud.fields[4].datatype == sensor_msgs::msg::PointField::INT32) {
    return InputPointType::XYZII;
  }

  return InputPointType::UNKNOWN;
}

uint8_t UniversalPointTransformerComponent::extractIntensityFromXYZI(const uint8_t * point_data)
{
  // For XYZI, intensity is a float at offset 12
  float intensity;
  memcpy(&intensity, point_data + 12, sizeof(float));

  // Scale from float (typically 0.0-1.0 or larger) to uint8 (0-255)
  return static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, intensity * 255.0f)));
}

uint8_t UniversalPointTransformerComponent::extractIntensityFromXYZRGB(const uint8_t * point_data)
{
  // For XYZRGB, rgb is stored as a float at offset 12
  float rgb_float;
  memcpy(&rgb_float, point_data + 12, sizeof(float));

  // Convert the float to a uint32 to access individual bytes
  uint32_t rgb = *reinterpret_cast<uint32_t *>(&rgb_float);

  // Extract individual RGB values
  uint8_t r = (rgb >> 16) & 0xFF;
  uint8_t g = (rgb >> 8) & 0xFF;
  uint8_t b = rgb & 0xFF;

  // Convert RGB to grayscale intensity using standard luminance formula
  // I = 0.299*R + 0.587*G + 0.114*B
  return static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
}

bool UniversalPointTransformerComponent::isCompatibleWithPointXYZIRC(
  const sensor_msgs::msg::PointCloud2 & cloud)
{
  using PointIndex = autoware::point_types::PointXYZIRCIndex;
  using PointXYZIRC = autoware::point_types::PointXYZIRC;

  if (cloud.fields.size() < 6) {
    return false;
  }

  bool is_compatible = true;

  // Check x field
  const auto & field_x = cloud.fields.at(static_cast<size_t>(PointIndex::X));
  is_compatible &= field_x.name == "x";
  is_compatible &= field_x.offset == offsetof(PointXYZIRC, x);
  is_compatible &= field_x.datatype == sensor_msgs::msg::PointField::FLOAT32;
  is_compatible &= field_x.count == 1;

  // Check y field
  const auto & field_y = cloud.fields.at(static_cast<size_t>(PointIndex::Y));
  is_compatible &= field_y.name == "y";
  is_compatible &= field_y.offset == offsetof(PointXYZIRC, y);
  is_compatible &= field_y.datatype == sensor_msgs::msg::PointField::FLOAT32;
  is_compatible &= field_y.count == 1;

  // Check z field
  const auto & field_z = cloud.fields.at(static_cast<size_t>(PointIndex::Z));
  is_compatible &= field_z.name == "z";
  is_compatible &= field_z.offset == offsetof(PointXYZIRC, z);
  is_compatible &= field_z.datatype == sensor_msgs::msg::PointField::FLOAT32;
  is_compatible &= field_z.count == 1;

  // Check intensity field
  const auto & field_intensity = cloud.fields.at(static_cast<size_t>(PointIndex::Intensity));
  is_compatible &= field_intensity.name == "intensity";
  is_compatible &= field_intensity.offset == offsetof(PointXYZIRC, intensity);
  is_compatible &= field_intensity.datatype == sensor_msgs::msg::PointField::UINT8;
  is_compatible &= field_intensity.count == 1;

  // Check return_type field
  const auto & field_return_type = cloud.fields.at(static_cast<size_t>(PointIndex::ReturnType));
  is_compatible &= field_return_type.name == "return_type";
  is_compatible &= field_return_type.offset == offsetof(PointXYZIRC, return_type);
  is_compatible &= field_return_type.datatype == sensor_msgs::msg::PointField::UINT8;
  is_compatible &= field_return_type.count == 1;

  // Check channel field
  const auto & field_channel = cloud.fields.at(static_cast<size_t>(PointIndex::Channel));
  is_compatible &= field_channel.name == "channel";
  is_compatible &= field_channel.offset == offsetof(PointXYZIRC, channel);
  is_compatible &= field_channel.datatype == sensor_msgs::msg::PointField::UINT16;
  is_compatible &= field_channel.count == 1;

  return is_compatible;
}

sensor_msgs::msg::PointCloud2 UniversalPointTransformerComponent::transform_to_xyzirc_compatible(
  const sensor_msgs::msg::PointCloud2 & input_cloud)
{
  using PointIndex = autoware::point_types::PointXYZIRCIndex;
  using PointXYZIRC = autoware::point_types::PointXYZIRC;
  // Create a new point cloud message
  sensor_msgs::msg::PointCloud2 output_cloud;

  // Copy header and basic properties
  output_cloud.header = input_cloud.header;
  output_cloud.height = input_cloud.height;
  output_cloud.width = input_cloud.width;
  output_cloud.is_bigendian = input_cloud.is_bigendian;
  output_cloud.is_dense = input_cloud.is_dense;

  // Detect input point type
  InputPointType input_type = detectInputPointType(input_cloud);

  if (input_type == InputPointType::UNKNOWN) {
    RCLCPP_WARN(get_logger(), "Unknown input point cloud format, attempting to process anyway");
  }

  // Clear and set up fields for PointXYZIRC
  output_cloud.fields.clear();

  // X field
  sensor_msgs::msg::PointField x_field;
  x_field.name = "x";
  x_field.offset = offsetof(PointXYZIRC, x);
  x_field.datatype = sensor_msgs::msg::PointField::FLOAT32;
  x_field.count = 1;
  output_cloud.fields.push_back(x_field);

  // Y field
  sensor_msgs::msg::PointField y_field;
  y_field.name = "y";
  y_field.offset = offsetof(PointXYZIRC, y);
  y_field.datatype = sensor_msgs::msg::PointField::FLOAT32;
  y_field.count = 1;
  output_cloud.fields.push_back(y_field);

  // Z field
  sensor_msgs::msg::PointField z_field;
  z_field.name = "z";
  z_field.offset = offsetof(PointXYZIRC, z);
  z_field.datatype = sensor_msgs::msg::PointField::FLOAT32;
  z_field.count = 1;
  output_cloud.fields.push_back(z_field);

  // Intensity field (converted to UINT8)
  sensor_msgs::msg::PointField intensity_field;
  intensity_field.name = "intensity";
  intensity_field.offset = offsetof(PointXYZIRC, intensity);
  intensity_field.datatype = sensor_msgs::msg::PointField::UINT8;
  intensity_field.count = 1;
  output_cloud.fields.push_back(intensity_field);

  // Return Type field
  sensor_msgs::msg::PointField return_type_field;
  return_type_field.name = "return_type";
  return_type_field.offset = offsetof(PointXYZIRC, return_type);
  return_type_field.datatype = sensor_msgs::msg::PointField::UINT8;
  return_type_field.count = 1;
  output_cloud.fields.push_back(return_type_field);

  // Channel field
  sensor_msgs::msg::PointField channel_field;
  channel_field.name = "channel";
  channel_field.offset = offsetof(PointXYZIRC, channel);
  channel_field.datatype = sensor_msgs::msg::PointField::UINT16;
  channel_field.count = 1;
  output_cloud.fields.push_back(channel_field);

  // Set point step to match PointXYZIRC size
  output_cloud.point_step = sizeof(PointXYZIRC);
  output_cloud.row_step = output_cloud.point_step * output_cloud.width;

  // Allocate data array
  output_cloud.data.resize(output_cloud.row_step * output_cloud.height);

  // Process each point in the input cloud
  for (size_t i = 0; i < input_cloud.width * input_cloud.height; ++i) {
    // Find position of current point in input data
    size_t input_offset = i * input_cloud.point_step;
    size_t output_offset = i * output_cloud.point_step;

    // Copy XYZ data (assuming same format)
    for (size_t j = 0; j < 12; ++j) {  // 3 * sizeof(float)
      output_cloud.data[output_offset + j] = input_cloud.data[input_offset + j];
    }

    // Extract and convert intensity based on input type
    uint8_t intensity_value = 0;
    if (input_type == InputPointType::XYZI || input_type == InputPointType::XYZII) {
      intensity_value = extractIntensityFromXYZI(&input_cloud.data[input_offset]);
    } else if (input_type == InputPointType::XYZRGB) {
      intensity_value = extractIntensityFromXYZRGB(&input_cloud.data[input_offset]);
    } else {
      // Unknown format - try to extract as XYZI first, then XYZRGB if that fails
      try {
        intensity_value = extractIntensityFromXYZI(&input_cloud.data[input_offset]);
      } catch (...) {
        try {
          intensity_value = extractIntensityFromXYZRGB(&input_cloud.data[input_offset]);
        } catch (...) {
          intensity_value = 128;  // Default to mid-range if all else fails
        }
      }
    }

    // Copy the intensity value
    memcpy(
      &output_cloud.data[output_offset + offsetof(PointXYZIRC, intensity)], &intensity_value,
      sizeof(uint8_t));

    // Set default values for return_type and channel
    uint8_t default_return_type = 0;  // Default return type
    uint16_t default_channel = 0;     // Default channel

    // Determine channel from row position for organized point clouds
    if (input_cloud.height > 1) {
      // For organized point clouds, use row number as channel
      size_t row = i / input_cloud.width;
      default_channel = static_cast<uint16_t>(row % UINT16_MAX);
    }

    memcpy(
      &output_cloud.data[output_offset + offsetof(PointXYZIRC, return_type)], &default_return_type,
      sizeof(uint8_t));
    memcpy(
      &output_cloud.data[output_offset + offsetof(PointXYZIRC, channel)], &default_channel,
      sizeof(uint16_t));
  }

  return output_cloud;
}

}  // namespace universal_point_transformer

RCLCPP_COMPONENTS_REGISTER_NODE(universal_point_transformer::UniversalPointTransformerComponent)
