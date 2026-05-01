#!/bin/bash

# Trap signals to ensure cleanup
trap cleanup EXIT
trap cleanup SIGINT
trap cleanup SIGTERM

# Function to clean up processes
cleanup() {
    echo "Cleaning up tmux session..."
    if [ -n "$SESSION_NAME" ]; then
        tmux kill-session -t "$SESSION_NAME" 2>/dev/null
        echo "Killed tmux session: $SESSION_NAME"
    fi
    # Optionally, kill the terminal running tmux if needed
    if [ -n "$terminal_pid" ]; then
        kill "$terminal_pid" 2>/dev/null
        echo "Killed terminal process: $terminal_pid"
    fi
}

MAP_PATH=""
SESSION_NAME="AW_navigation_stack"

function print_options() {
    echo
    echo "Choose an option:"
    echo "  [r] (Reload) Loads a new map"
    echo "  [q] Quit "
}


function load_map_path() {
    while true; do
        MAP_PATH=$(zenity --file-selection --directory --title="Select a map directory")

        if [[ -n "$MAP_PATH" ]]; then
            echo "You selected: $MAP_PATH"
            break
        else
            zenity --warning --text="No map selected. Please try again." --title="Selection Required"
        fi
    done
}


function start_stack()
{
    tmux new-session -d -s ${SESSION_NAME}

    tmux rename-window -t $SESSION_NAME:0 "Vehicle Interface - Mock"
    tmux send-keys -t ${SESSION_NAME}:0 "ros2 launch rt_vehicle_interface rt_vehicle_interface.launch.xml" C-m

    tmux new-window -t $SESSION_NAME -n "Planning"
    tmux send-keys -t ${SESSION_NAME}:1 "ros2 launch gd_twizy_planning twizy_planning.launch.xml" C-m

    tmux new-window -t $SESSION_NAME -n "Localization"
    tmux send-keys -t ${SESSION_NAME}:2 "ros2 launch rt_localization_launcher rt_localization.launch.xml" C-m

    tmux new-window -t $SESSION_NAME -n "Map"
    tmux send-keys -t ${SESSION_NAME}:3 "ros2 launch rt_map_launch rt_map.launch.xml map_path:=${MAP_PATH}" C-m

    tmux new-window -t $SESSION_NAME -n "Perception"
    tmux send-keys -t ${SESSION_NAME}:4 "ros2 launch rt_perception_launcher twizy_perception.launch.xml map_path:=${MAP_PATH}" C-m

    tmux new-window -t $SESSION_NAME -n "Sensing"
    tmux send-keys -t ${SESSION_NAME}:5 "ros2 launch gd_twizy_sensor_launch sensing.launch.xml" C-m

    tmux new-window -t $SESSION_NAME -n "RPi-Vehicle Controls"
    tmux send-keys -t ${SESSION_NAME}:6 "sshpass -p '0' ssh -tt roos@192.168.1.100 'bash -s' < ./pi_vehicle_control_ros.sh" C-m

    tmux new-window -t $SESSION_NAME -n "RPi-Laserscanners"
    tmux send-keys -t ${SESSION_NAME}:7 "sshpass -p '0' ssh -tt roos@192.168.1.100 'bash -s' < ./laserscanners.sh" C-m

    tmux new-window -t $SESSION_NAME -n "Engage mock"
    tmux send-keys -t ${SESSION_NAME}:8 "ros2 topic pub /autoware/engage autoware_vehicle_msgs/msg/Engage '{engage: true}'" C-m

    tmux new-window -t $SESSION_NAME -n "RT-System"
    tmux send-keys -t ${SESSION_NAME}:9 "ros2 launch rt_system_launcher twizy_system.launch.xml" C-m

    tmux new-window -t $SESSION_NAME -n "Control"
    tmux send-keys -t ${SESSION_NAME}:10 "ros2 launch gd_twizy_control twizy_control.launch.xml" C-m

    x-terminal-emulator -e "tmux attach"
}


function run_r_action() {
    echo "You pressed R — reloading map..."
    load_map_path
    tmux send-keys -t $SESSION_NAME:3 C-c
    tmux send-keys -t $SESSION_NAME:4 C-c
    sleep 2
    tmux send-keys -t ${SESSION_NAME}:3 "ros2 launch gd_map_launch gd_map.launch.xml map_path:=${MAP_PATH}" C-m
    tmux send-keys -t ${SESSION_NAME}:4 "ros2 launch rt_perception_launcher twizy_perception.launch.xml map_path:=${MAP_PATH}" C-m

}

function run_q_action() {
    echo "You pressed Q — shutting down..."
    tmux kill-session -t "${SESSION_NAME}"
    while true; do
    	output=$(ros2 node list)

    if [ -n "$output" ]; then
    	if [ "$shutting_down" = false ]; then
            echo -n "Shutting down"
            shutting_down=true
        else
            echo -n "."
        fi
    else
        echo "Shutdown complete."
        break
    fi

    sleep 1
done
    exit 0
}


load_map_path
print_options
start_stack

terminal_pid=$!

while true; do
    read -n 1 -s key
    case "$key" in
        r|R)
            run_r_action
            ;;
        q|Q)
            run_q_action
            ;;
        *)
            print_options
            ;;
    esac
done

exit 0
