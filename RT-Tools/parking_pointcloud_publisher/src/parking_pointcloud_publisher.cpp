#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>

#include <Eigen/Dense>
#include <pcl_ros/transforms.hpp>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"

class PointCloudPublisher : public rclcpp::Node {
 public:
  PointCloudPublisher() : Node("pointcloud_publisher") {
    publisher_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(
        "/perception/obstacle_segmentation/pointcloud", 10);
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(300),
        std::bind(&PointCloudPublisher::publishPointCloud, this));

    this->declare_parameter<float>("length", 1.0);
    this->get_parameter(
        "length",
        length_);  // TODO make a function that checks for existence of
                   // parameter and notifies when the default is used
    width_ = declare_parameter<float>("width");
    height_ = declare_parameter<float>("height");
    num_points_per_edge_ = declare_parameter<int>("number_of_points_per_edge");
    frame_ = declare_parameter<std::string>("frame");
    theta_ = declare_parameter<double>("theta");
    translation_.resize(3);
    this->declare_parameter<std::vector<double>>(
        "translation", std::vector<double>{0.0, 0.0, 0.0});
    this->get_parameter("translation", translation_);
    cloud_ =
        pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>);
    create3DRectangleWithSideMissing();
  }

 private:
  void create3DRectangleWithSideMissing() {
    float step_length = length_ / num_points_per_edge_;
    float step_width = width_ / num_points_per_edge_;
    float step_height = height_ / num_points_per_edge_;

    cloud_->clear();
    // Front face (length x height)
    for (int i = 0; i <= num_points_per_edge_; ++i) {
      for (int j = 0; j <= num_points_per_edge_; ++j) {
        cloud_->points.push_back(
            pcl::PointXYZ(i * step_length, 0, j * step_height));
      }
    }

    // Back face (length x height)
    for (int i = 0; i <= num_points_per_edge_; ++i) {
      for (int j = 0; j <= num_points_per_edge_; ++j) {
        cloud_->points.push_back(
            pcl::PointXYZ(i * step_length, width_, j * step_height));
      }
    }

    // Left face (width x height)
    for (int i = 0; i <= num_points_per_edge_; ++i) {
      for (int j = 0; j <= num_points_per_edge_; ++j) {
        cloud_->points.push_back(
            pcl::PointXYZ(0, i * step_width, j * step_height));
      }
    }

    cloud_->width = cloud_->size();
    cloud_->height = 1;
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    transform(0, 3) = translation_[0];  // Translation in x
    transform(1, 3) = translation_[1];  // Translation in y
    transform(2, 3) = translation_[2];  // Translation in z
    transform(0, 0) = cos(theta_);
    transform(0, 1) = -sin(theta_);
    transform(1, 0) = sin(theta_);
    transform(1, 1) = cos(theta_);
    pcl::transformPointCloud(*cloud_, *cloud_, transform);

    pcl::toROSMsg(*cloud_, cloud_msg_);
  }

  void publishPointCloud() {
    cloud_msg_.header.stamp = this->now();
    cloud_msg_.header.frame_id = frame_;
    publisher_->publish(cloud_msg_);
  }

  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_;
  sensor_msgs::msg::PointCloud2 cloud_msg_;
  float length_;
  float width_;
  float height_;
  int num_points_per_edge_;
  std::string frame_;
  std::vector<double> translation_;
  double theta_;
};

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PointCloudPublisher>());
  rclcpp::shutdown();
  return 0;
}
