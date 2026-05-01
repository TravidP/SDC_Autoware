#include <fstream>
#include <iostream>
#include <sl/Camera.hpp>

using namespace sl;
using namespace std;

void print(string msg_prefix, ERROR_CODE err_code = ERROR_CODE::SUCCESS, string msg_suffix = "");

int main(int argc, char ** argv)
{
  if (argc <= 1) {
    cout << "Usage: \n";
    cout << "$ SVO_Timestamps <SVO_file> \n";
    cout << "  ** SVO file is mandatory in the application ** \n\n";
    return EXIT_FAILURE;
  }

  // Create ZED objects
  Camera zed;
  InitParameters init_parameters;
  init_parameters.input.setFromSVOFile(argv[1]);
  init_parameters.depth_mode = sl::DEPTH_MODE::PERFORMANCE;

  // Open the camera
  auto returned_state = zed.open(init_parameters);
  if (returned_state != ERROR_CODE::SUCCESS) {
    print("Camera Open", returned_state, "Exit program.");
    return EXIT_FAILURE;
  }

  int nb_frames = zed.getSVONumberOfFrames();

  ofstream timestamps_file;
  std::string filename = argv[1];
  timestamps_file.open(filename + ".timestamps");

  int expected_svo_position = 0;
  cout << "starting export of timestamps; total number of frames: " << nb_frames << "\n";
  while (returned_state <= ERROR_CODE::SUCCESS) {
    returned_state = zed.grab();
    int svo_position = zed.getSVOPosition();
    if (svo_position != expected_svo_position) {
      break;
    }
    timestamps_file << zed.getTimestamp(TIME_REFERENCE::IMAGE) << "\n";
    expected_svo_position++;
  }
  cout << "exported timestamps for the first " << expected_svo_position << " frames\n";
  timestamps_file.close();

  zed.close();
  return EXIT_SUCCESS;
}

void print(string msg_prefix, ERROR_CODE err_code, string msg_suffix)
{
  cout << "[Sample]";
  if (err_code != ERROR_CODE::SUCCESS)
    cout << "[Error] ";
  else
    cout << " ";
  cout << msg_prefix << " ";
  if (err_code != ERROR_CODE::SUCCESS) {
    cout << " | " << toString(err_code) << " : ";
    cout << toVerbose(err_code);
  }
  if (!msg_suffix.empty()) cout << " " << msg_suffix;
  cout << endl;
}
