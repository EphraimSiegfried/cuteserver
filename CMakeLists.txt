cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set the project name and version
project(cuteserver VERSION 1.0 LANGUAGES C)

# Specify the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add compiler flags
add_compile_options(-Wall -DLOG_USE_COLOR)
add_link_options(-pthread)

include_directories(deps)

# Define the target executable and its source files
file(GLOB_RECURSE SRC_FILES "src/*.c" "deps/*/*.c")
add_executable(${PROJECT_NAME} ${SRC_FILES})
install(TARGETS ${PROJECT_NAME})
