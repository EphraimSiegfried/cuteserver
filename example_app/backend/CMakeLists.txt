cmake_minimum_required(VERSION 3.10)

project(backend)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Prefer static libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

find_path(JSON_C_INCLUDE_DIR json-c/json.h)
find_library(JSON_C_LIBRARY json-c)

if(NOT JSON_C_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find json-c headers")
endif()

if(NOT JSON_C_LIBRARY)
    message(FATAL_ERROR "Could not find json-c library")
endif()

include_directories(${JSON_C_INCLUDE_DIR})

add_executable(main.cgi main.c)

target_link_libraries(main.cgi ${JSON_C_LIBRARY})
