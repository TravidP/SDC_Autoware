#include "SvoRosbagSyncNode.cpp"  // TODO: don't include cpp files (use CMake?)
#include "SvoRosbagSyncNode.hpp"
#include "TfTopicSync.cpp"  // TODO: don't include cpp files (use CMake?)
#include "TopicSync.cpp"    // TODO: don't include cpp files (use CMake?)
#include "TopicSync.hpp"
#include "TopicSyncAndConvert.cpp"  // TODO: don't include cpp files (use CMake?)
#include "rclcpp/rclcpp.hpp"

using namespace std;

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SvoRosbagSyncNode>());
  rclcpp::shutdown();

  return 0;
}