cmake_minimum_required(VERSION 3.8)
project(map_projector_generator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(pugixml QUIET)
if(NOT pugixml_FOUND)
  message(
    FATAL_ERROR
      "pugixml not found. Please install it and make sure it's in your CMAKE_PREFIX_PATH."
  )
endif()

find_package(yaml-cpp QUIET)
if(NOT yaml-cpp_FOUND)
  message(
    FATAL_ERROR
      "yaml-cpp not found. Please install it and make sure it's in your CMAKE_PREFIX_PATH."
  )
endif()

find_library(PUGIXML_LIBRARIES NAMES pugixml)

find_path(
  PUGIXML_INCLUDE_DIRS
  NAMES pugixml.hpp
  PATH_SUFFIXES pugixml)

# Add executable
add_executable(map_projector_generator src/map_projector_generator.cpp)

# Link libraries
target_link_libraries(map_projector_generator PRIVATE pugixml::pugixml yaml-cpp)

