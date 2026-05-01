#ifndef ROSBAG_RECORDER_HPP
#define ROSBAG_RECORDER_HPP

#include <atomic>
#include <boost/process.hpp>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct Result
{
   bool success_ = false;
   std::string msg_ = "";
};

class RosbagRecorder
{
public:
  /**
     * @brief Constructor for RosbagRecorder
     * @param bag_directory Directory to store rosbag files (default: system temp directory)
     */
  explicit RosbagRecorder(const std::string & bag_directory = "");

  /**
     * @brief Destructor - ensures recording is stopped
     */
  ~RosbagRecorder();

  /**
     * @brief Start recording a rosbag
     * @param topics Vector of topic names to record
     * @param bag_name Optional custom bag name (timestamp used if empty)
     * @return Result with success true or false, msg is populated with info
     */
  Result startRecording(const std::vector<std::string> & topics, const std::string & bag_name = "");

  /**
     * @brief Stop the current recording
     * @return Result with success true or false, msg is populated with info
     */
  Result stopRecording();

private:
  std::filesystem::path get_home_directory();
  std::string join_strings(const std::vector<std::string>& topic_vector);

  mutable std::mutex mtx_;
  std::unique_ptr<boost::process::child> process_;
  std::string bag_directory_;
  const std::string shell_command_;
  const std::string shell_option_;
};

#endif  // ROSBAG_RECORDER_HPP
