cmake_minimum_required(VERSION 3.8)
project(unity_ui_build)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# Run FTP download script
install(CODE "execute_process(COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/download_ftp.py ${CMAKE_INSTALL_PREFIX}/share/unity_ui_build)")

ament_auto_package(
        INSTALL_TO_SHARE
        launch
        shell_scripts
)
