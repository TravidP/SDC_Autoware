#include "rosbag_recorder/rosbag_recorder.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <thread>

namespace bp = boost::process;

RosbagRecorder::RosbagRecorder(const std::string & bag_directory)
:shell_command_("/usr/bin/bash"), shell_option_("-c")
{
  // Use temporary directory if none specified
  if (bag_directory.empty()) {
    bag_directory_ = get_home_directory();
  }
}

RosbagRecorder::~RosbagRecorder()
{
  if (process_ && process_->running()) {
    stopRecording();
  }
}

Result RosbagRecorder::startRecording(
  const std::vector<std::string> & topic_names, const std::string & output_bag_name)
{
  std::lock_guard<std::mutex> lock(mtx_);

  if (process_ && process_->running()) {
    std::string msg = "Recording process is already running with pid " + std::to_string(process_->id());
    return Result{false, msg};
  }

  try {
    std::string topics_arg;
    std::string output_path = bag_directory_ +  output_bag_name;

    // Build command based on whether specific topics are provided
    std::string command;
    if (topic_names.empty()) {
      // Record all topics if topic vector is empty
      command = "source /opt/ros/humble/setup.bash && /opt/ros/humble/bin/ros2 bag record -a -o " + output_path;
    } else {
      // Record specific topics
      topics_arg = join_strings(topic_names);
      command = "source /opt/ros/humble/setup.bash && /opt/ros/humble/bin/ros2 bag record " + topics_arg + " -o " + output_path;
    }

    process_ = std::make_unique<bp::child>(shell_command_, shell_option_, command,
                                          bp::std_out > bp::null,
                                          bp::std_err > bp::null);

    // Give the process a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if process is still running
    if (!process_->running()) {
      int exit_code = process_->exit_code();
      process_.reset();
      std::string msg = "Process exited immediately with code: " + std::to_string(exit_code);
      return Result{false, msg};
    }

    std::string msg = "Started process with pid: " + std::to_string(process_->id());
    return Result{true, msg};

  } catch (const std::exception & e) {
    process_.reset();
    std::string msg = std::string("Failed to start process: ") + std::string(e.what());
    return Result{false, msg};
  }
}

Result RosbagRecorder::stopRecording()
{
  std::lock_guard<std::mutex> lock(mtx_);

  if (process_ && process_->running()) {
    process_->terminate();
    process_->wait();
    std::string msg = "Process stopped with exit code: " + std::to_string(process_->exit_code());
    process_.reset();
    return Result{true, msg};
  } else {
    return Result{false, std::string("No recording process is running.")};
  }
}

std::filesystem::path RosbagRecorder::get_home_directory()
{
  const char * home = std::getenv("HOME");
  if (home) {
    return std::filesystem::path(home);
  } else {
    throw std::runtime_error("Home directory not found.");
  }
}

std::string RosbagRecorder::join_strings(const std::vector<std::string> & topic_vector)
{
  std::ostringstream oss;
  for (size_t i = 0; i < topic_vector.size(); ++i) {
    if (i != 0) oss << ' ';
    oss << topic_vector[i];
  }
  return oss.str();
}
