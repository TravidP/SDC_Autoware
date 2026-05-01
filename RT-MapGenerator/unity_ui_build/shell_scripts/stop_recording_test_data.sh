#!/bin/bash

# Find the process ID (PID) of the ros2 bag record process
PID=$(pgrep -f "ros2 bag record")

if [ -z "$PID" ]; then
  echo "No ros2 bag record process found."
else
  echo "Stopping ros2 bag record process with PID: $PID"
  # Send SIGINT (interrupt signal) to gracefully stop the process
  kill -INT $PID
fi
