#ifndef SESSION_RECORDER_HPP
#define SESSION_RECORDER_HPP

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <rosbag2_cpp/writer.hpp>
#include <rosbag2_storage/storage_options.hpp>
#include <rosbag2_cpp/writers/sequential_writer.hpp>

#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <filesystem>
#include <chrono>

class SessionRecorder : public rclcpp::Node
{
public:
    SessionRecorder();
    ~SessionRecorder();

private:
    // Parameters
    std::vector<std::string> topics_;
    std::vector<std::string> namespaces_;
    std::string output_directory_;
    std::string bag_name_prefix_;
    double space_threshold_gb_;
    double check_interval_seconds_;

    // Recording state
    std::atomic<bool> is_recording_;
    std::atomic<bool> should_stop_;
    std::unique_ptr<rosbag2_cpp::Writer> writer_;
    std::string current_bag_path_;

    // Monitoring
    std::thread monitoring_thread_;
    rclcpp::TimerBase::SharedPtr status_timer_;

    // Publishers and Services
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr start_service_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr stop_service_;
    std::vector<rclcpp::GenericSubscription::SharedPtr> subscriptions_;

    // Methods
    void loadParameters();
    void setupWriter();
    void startRecording();
    void stopRecording();
    void monitoringLoop();
    double getAvailableSpaceGB(const std::string& path);
    double getBagSizeGB(const std::string& bag_path);
    std::vector<std::string> getAllTopicsInNamespaces();
    std::vector<std::string> getTopicsToRecord();
    std::string generateBagName();
    void publishStatus();

    // Service callbacks
    void startRecordingCallback(
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response);

    void stopRecordingCallback(
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response);
};

#endif // SESSION_RECORDER_HPP
