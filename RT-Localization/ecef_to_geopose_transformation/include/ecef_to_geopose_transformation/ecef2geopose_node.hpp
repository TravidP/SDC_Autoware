#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <geographic_msgs/msg/geo_pose_with_covariance_stamped.hpp>
#include <std_msgs/msg/float64.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

class Ecef2GeoposeTransform : public rclcpp::Node
{
    public:
    Ecef2GeoposeTransform();
    private:
    // Helper functions
    constexpr double deg2rad(double deg) { return deg * M_PI / 180.0; };
    tf2::Matrix3x3 ECEFtoENURotation(double lat_deg, double lon_deg);

    void ecef_callback(const nav_msgs::msg::Odometry::SharedPtr ecef);
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr ecef_sub_;
    rclcpp::Publisher<geographic_msgs::msg::GeoPoseWithCovarianceStamped>::SharedPtr geopose_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr proc_time_pub_;
    std::string input_topic_;
    std::string output_topic_;
};
