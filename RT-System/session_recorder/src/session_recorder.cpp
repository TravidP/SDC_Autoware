#include "session_recorder/session_recorder.hpp"

#include <filesystem>
#include <iomanip>
#include <sstream>

SessionRecorder::SessionRecorder()
: Node("session_recorder"), is_recording_(false), should_stop_(false)
{
  RCLCPP_INFO(this->get_logger(), "Initializing Smart Bag Recorder");

  loadParameters();

  status_pub_ = this->create_publisher<std_msgs::msg::String>("~/status", 10);

  start_service_ = this->create_service<std_srvs::srv::Trigger>(
    "~/start_recording", std::bind(
                           &SessionRecorder::startRecordingCallback, this, std::placeholders::_1,
                           std::placeholders::_2));

  stop_service_ = this->create_service<std_srvs::srv::Trigger>(
    "~/stop_recording",
    std::bind(
      &SessionRecorder::stopRecordingCallback, this, std::placeholders::_1, std::placeholders::_2));

  status_timer_ = this->create_wall_timer(
    std::chrono::seconds(5), std::bind(&SessionRecorder::publishStatus, this));

  monitoring_thread_ = std::thread(&SessionRecorder::monitoringLoop, this);

  RCLCPP_INFO(this->get_logger(), "Smart Bag Recorder initialized");

  bool auto_start = this->declare_parameter("auto_start", false);
  if (auto_start) {
    RCLCPP_INFO(this->get_logger(), "Auto-starting recording...");
    startRecording();
  }
}

SessionRecorder::~SessionRecorder()
{
  should_stop_ = true;
  if (is_recording_) {
    stopRecording();
  }
  if (monitoring_thread_.joinable()) {
    monitoring_thread_.join();
  }
}

void SessionRecorder::loadParameters()
{
  // Required parameters - no defaults, will throw if missing from config file
  topics_ = this->declare_parameter<std::vector<std::string>>("topics");
  namespaces_ = this->declare_parameter<std::vector<std::string>>("namespaces");
  output_directory_ = this->declare_parameter<std::string>("output_directory");

  if (topics_.empty() && namespaces_.empty()) {
    RCLCPP_ERROR(
      this->get_logger(), "Both topics and namespaces are empty. At least one must be specified.");
    rclcpp::shutdown();
    return;
  }

  if (output_directory_.empty()) {
    RCLCPP_ERROR(this->get_logger(), "output_directory cannot be empty");
    rclcpp::shutdown();
    return;
  }

  bag_name_prefix_ = this->declare_parameter<std::string>("bag_name_prefix");
  space_threshold_gb_ = this->declare_parameter<double>("space_threshold_gb");
  check_interval_seconds_ = this->declare_parameter<double>("check_interval_seconds");

  try {
    std::filesystem::create_directories(output_directory_);
  } catch (const std::exception & e) {
    RCLCPP_ERROR(
      this->get_logger(), "Failed to create output directory '%s': %s", output_directory_.c_str(),
      e.what());
    rclcpp::shutdown();
    return;
  }

  RCLCPP_INFO(this->get_logger(), "Loaded parameters:");
  RCLCPP_INFO(this->get_logger(), "  Specific topics: %zu", topics_.size());
  RCLCPP_INFO(this->get_logger(), "  Namespaces (all topics): %zu", namespaces_.size());
  RCLCPP_INFO(this->get_logger(), "  Output directory: %s", output_directory_.c_str());
  RCLCPP_INFO(this->get_logger(), "  Space threshold: %.2f GB", space_threshold_gb_);
}

std::string SessionRecorder::generateBagName()
{
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  std::stringstream ss;
  ss << bag_name_prefix_ << "_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << "_"
     << std::setfill('0') << std::setw(3) << ms.count();

  return ss.str();
}

void SessionRecorder::startRecording()
{
  if (is_recording_) {
    RCLCPP_WARN(this->get_logger(), "Already recording");
    return;
  }

  double available_space = getAvailableSpaceGB(output_directory_);
  if (available_space < space_threshold_gb_) {
    RCLCPP_ERROR(
      this->get_logger(),
      "Insufficient space to start recording. Available: %.2f GB, Required: %.2f GB",
      available_space, space_threshold_gb_);
    return;
  } else {
    RCLCPP_INFO(
      this->get_logger(), "Maximum available space %.2f GB, Required: %.2f GB", available_space,
      space_threshold_gb_);
  }

  try {
    setupWriter();

    // Get topics to record (either specific topics or all topics in namespaces)
    auto topics_to_record = getTopicsToRecord();

    if (topics_to_record.empty()) {
      RCLCPP_ERROR(this->get_logger(), "No topics found to record!");
      return;
    }

    RCLCPP_INFO(this->get_logger(), "Starting recording with %zu topics:", topics_to_record.size());
    for (const auto & topic : topics_to_record) {
      RCLCPP_INFO(this->get_logger(), "  - %s", topic.c_str());
    }

    // Create topic list for rosbag2
    std::vector<std::string> topic_names = topics_to_record;

    is_recording_ = true;
    RCLCPP_INFO(this->get_logger(), "Recording started: %s", current_bag_path_.c_str());

    subscriptions_.clear();

    for (const auto & topic_name : topics_to_record) {
      auto info = this->get_topic_names_and_types();
      if (info.find(topic_name) == info.end()) {
        RCLCPP_WARN(this->get_logger(), "Topic %s not available", topic_name.c_str());
        continue;
      }

      auto type_name = info[topic_name][0];
      RCLCPP_INFO(
        this->get_logger(), "Subscribing to %s [%s]", topic_name.c_str(), type_name.c_str());

      // Register topic with writer
      rosbag2_storage::TopicMetadata metadata;
      metadata.name = topic_name;
      metadata.type = type_name;
      metadata.serialization_format = "cdr";
      writer_->create_topic(metadata);

      // Use generic subscription so you don’t need to hardcode types
      auto sub = this->create_generic_subscription(
        topic_name, type_name, rclcpp::QoS(10),
        [this, topic_name, type_name](std::shared_ptr<rclcpp::SerializedMessage> msg) {
          writer_->write(msg, topic_name, type_name, this->get_clock()->now());
        });

      subscriptions_.push_back(sub);
    }

  } catch (const std::exception & e) {
    RCLCPP_ERROR(this->get_logger(), "Failed to start recording: %s", e.what());
    is_recording_ = false;
  }
}

void SessionRecorder::setupWriter()
{
  current_bag_path_ = output_directory_ + "/" + generateBagName();

  rosbag2_storage::StorageOptions storage_options;
  storage_options.uri = current_bag_path_;
  storage_options.storage_id = "sqlite3";

  rosbag2_cpp::ConverterOptions converter_options;
  converter_options.input_serialization_format = "cdr";
  converter_options.output_serialization_format = "cdr";

  writer_ = std::make_unique<rosbag2_cpp::Writer>();
  writer_->open(storage_options, converter_options);
}

void SessionRecorder::stopRecording()
{
  if (!is_recording_) {
    RCLCPP_WARN(this->get_logger(), "Not currently recording");
    return;
  }

  try {
    if (writer_) {
      writer_->close();
      writer_.reset();
    }

    is_recording_ = false;

    double bag_size = getBagSizeGB(current_bag_path_);
    RCLCPP_INFO(
      this->get_logger(), "Recording stopped. Bag size: %.2f GB, Location: %s", bag_size,
      current_bag_path_.c_str());

  } catch (const std::exception & e) {
    RCLCPP_ERROR(this->get_logger(), "Error stopping recording: %s", e.what());
    is_recording_ = false;
  }
}

void SessionRecorder::monitoringLoop()
{
  while (!should_stop_) {
    if (is_recording_) {
      double available_space = getAvailableSpaceGB(output_directory_);
      double current_bag_size = getBagSizeGB(current_bag_path_);

      if (available_space < space_threshold_gb_) {
        RCLCPP_WARN(
          this->get_logger(), "Low disk space detected! Available: %.2f GB, Threshold: %.2f GB",
          available_space, space_threshold_gb_);
        RCLCPP_WARN(this->get_logger(), "Stopping recording to prevent disk full");
        stopRecording();
      } else {
        RCLCPP_DEBUG(
          this->get_logger(), "Space check - Available: %.2f GB, Bag size: %.2f GB",
          available_space, current_bag_size);
      }
    }

    std::this_thread::sleep_for(
      std::chrono::milliseconds(static_cast<int>(check_interval_seconds_ * 1000)));
  }
}

double SessionRecorder::getAvailableSpaceGB(const std::string & path)
{
  try {
    auto space_info = std::filesystem::space(path);
    return static_cast<double>(space_info.available) / (1024.0 * 1024.0 * 1024.0);
  } catch (const std::exception & e) {
    RCLCPP_ERROR(this->get_logger(), "Error checking disk space: %s", e.what());
    return 0.0;
  }
}

double SessionRecorder::getBagSizeGB(const std::string & bag_path)
{
  try {
    if (!std::filesystem::exists(bag_path)) {
      return 0.0;
    }

    std::uintmax_t size = 0;
    for (const auto & entry : std::filesystem::recursive_directory_iterator(bag_path)) {
      if (entry.is_regular_file()) {
        size += entry.file_size();
      }
    }

    return static_cast<double>(size) / (1024.0 * 1024.0 * 1024.0);
  } catch (const std::exception & e) {
    RCLCPP_ERROR(this->get_logger(), "Error calculating bag size: %s", e.what());
    return 0.0;
  }
}

void SessionRecorder::publishStatus()
{
  std_msgs::msg::String status_msg;

  if (is_recording_) {
    double available_space = getAvailableSpaceGB(output_directory_);
    double bag_size = getBagSizeGB(current_bag_path_);

    std::stringstream ss;
    ss << "RECORDING - Bag: " << std::filesystem::path(current_bag_path_).filename().string()
       << ", Size: " << std::fixed << std::setprecision(2) << bag_size << " GB"
       << ", Available: " << available_space << " GB";
    status_msg.data = ss.str();
  } else {
    status_msg.data = "IDLE - Not recording";
  }

  status_pub_->publish(status_msg);
}

void SessionRecorder::startRecordingCallback(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  (void)request;
  if (is_recording_) {
    response->success = false;
    response->message = "Already recording";
  } else {
    startRecording();
    response->success = is_recording_;
    response->message =
      is_recording_ ? "Recording started: " + current_bag_path_ : "Failed to start recording";
  }
}

void SessionRecorder::stopRecordingCallback(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  (void)request;

  if (!is_recording_) {
    response->success = false;
    response->message = "Not currently recording";
  } else {
    std::string bag_path = current_bag_path_;
    stopRecording();
    response->success = !is_recording_;
    response->message =
      response->success ? "Recording stopped: " + bag_path : "Failed to stop recording";
  }
}

std::vector<std::string> SessionRecorder::getTopicsToRecord()
{
  std::vector<std::string> topics_to_record = topics_;  // start with explicit topics

  // Discover all topics from ROS2
  auto all_topics = this->get_topic_names_and_types();

  // For each namespace, add all topics that start with that namespace
  for (const auto & ns : namespaces_) {
    for (const auto & [name, types] : all_topics) {
      if (name.rfind(ns, 0) == 0) {  // name starts with namespace
        // Avoid duplicates
        if (
          std::find(topics_to_record.begin(), topics_to_record.end(), name) ==
          topics_to_record.end()) {
          topics_to_record.push_back(name);
        }
      }
    }
  }

  return topics_to_record;
}
