// universal_point_transformer.hpp
#ifndef UNIVERSAL_POINT_TRANSFORMER_HPP_
#define UNIVERSAL_POINT_TRANSFORMER_HPP_

#include <autoware/point_types/types.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace universal_point_transformer
{

enum class InputPointType
{
  UNKNOWN,
  XYZI,   // x,y,z as float32 + intensity as float32
  XYZII,  // x,y,z,i as float32 + index as int32
  XYZRGB  // x,y,z as float32 + rgb as float32
};

class UniversalPointTransformerComponent : public rclcpp::Node
{
public:
  explicit UniversalPointTransformerComponent(const rclcpp::NodeOptions & options);

private:
  void pointCloudCallback(const sensor_msgs::msg::PointCloud2::SharedPtr msg);

  sensor_msgs::msg::PointCloud2 transform_to_xyzirc_compatible(
    const sensor_msgs::msg::PointCloud2 & input_cloud);

  InputPointType detectInputPointType(const sensor_msgs::msg::PointCloud2 & cloud);

  uint8_t extractIntensityFromXYZI(const uint8_t * point_data);
  uint8_t extractIntensityFromXYZRGB(const uint8_t * point_data);

  bool isCompatibleWithPointXYZIRC(const sensor_msgs::msg::PointCloud2 & cloud);

  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr subscription_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr publisher_;
};

}  // namespace universal_point_transformer

#endif  // UNIVERSAL_POINT_TRANSFORMER_HPP_
