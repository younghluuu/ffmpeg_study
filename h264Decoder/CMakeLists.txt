cmake_minimum_required(VERSION 3.5)
project(h264Decode)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_include_directories(${PROJECT_NAME} PRIVATE .)