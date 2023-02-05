cmake_minimum_required(VERSION 3.5)
project(udp-rtp)

set(CMAKE_C_STANDARD 11)

add_executable(${PROJECT_NAME} main.c)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif ()