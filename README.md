# RT-Autoware
This is the top level repository for the packages (configurations, scripts etc.) related to Autoware and our specific hardware configuration.

## Setup
1. Clone this repo: `git clone git@github.com:robotTUNER/RT-Autoware.git`\
If this fails, make sure a private key for github is loaded: see [here](https://docs.github.com/en/authentication/troubleshooting-ssh/error-permission-denied-publickey#make-sure-you-have-a-key-that-is-being-used)

2. Run `cd RT-Autoware` && `mkdir src`.

3. To clone the repos `vcs import src < robottuner.repos` to update the repos `vcs pull src` (note that the pull command does not switch branches).

4. Build the workspace using `colcon build --symlink-install`.

5. If the dependent related repositories are not present, use `vcs import --recursive src < dependencies.repos` to pull the necessary repos. Note these repos need to be build separately. Consult their github pages.
   
   Notes for zed-ros2-wrapper:
   
   - This repository has a submodule `zed-ros2-interfaces`, if the submodule folder is empty make sure you used the `--recursive` tag while importing or cloning the `zed-ros2-wrapper` repository.
   - If not all dependencies have been installed you may need to run the following commands before building with colcon:
     ```
     sudo apt update
     rosdep update
     rosdep install --from-paths src --ignore-src -r -y # install dependencies
     ```
   - See [here](https://github.com/stereolabs/zed-ros2-wrapper) for more information.

   Notes for fixposition_driver installation (GTest installation might be required):

    - Read the [following](https://docs.fixposition.com/fd/installation-and-usage)
    - Use `colcon build --base-paths ./src/fixposition_driver/ --cmake-args -DFPSDK_ENABLE_TESTS=OFF -DBUILD_TESTING=OFF` if build failing due to GTest issues.

6. Make sure the right version of ZED SDK is installed so zed ROS2 dependencies can be built without issue.

7. Run `./utility_scripts/setup_hooks.sh` to make all the repos under src use the hook scripts in .githooks.

## Launch
1. It is recommended to permanently source .tmux.conf to enable mouse support for tmux by adding the following to the terminal's .*rc.
(Note: tmux is necessary for launching the stack using the scripts)

    ```
    if [ -n "$TMUX" ]; then

    tmux source-file ~/RT-Autoware/.tmux.conf

    fi
    ````

2. Run stack_dev_launch_twizy_ros.sh for the whole stack to launch. Funtionality is split in different tmux windows (with relevant names). The script is interactive and expects input from the user to shut down the running stack. Follow the terminal instructions. (Note: the map reloading functionality is still under construction, to load a new map relaunch the whole stack.)

3. Creating a new route by driving can be accomplished by running route_recording_functionality.sh the navigation stack **must be running**.

## Utility Scripts

- repo_state_check.sh

  When run it informs the user of the state of all the repos under src. You can provide it a path to check, for repos or you can run it without arguments and it will check under src folder of RT-Autoware.

## TODO

- [ ] Add --cmake-args -DCMAKE_BUILD_TYPE=Release for building ?

- [ ] Update naming and path for vehicle_info_util.

- [ ] Separate where the dependencies are built.
