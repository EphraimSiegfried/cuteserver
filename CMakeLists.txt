cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(cuteserver VERSION 1.0)

# Specify the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ideps -Wall -DLOG_USE_COLOR -pthread")

include_directories(deps)

# Define the target executable and its source files
file(GLOB_RECURSE SRC_FILES "src/*.c" "deps/*/*.c")
add_executable(${PROJECT_NAME} ${SRC_FILES})



