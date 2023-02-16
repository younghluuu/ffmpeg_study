cmake_minimum_required(VERSION 3.5)
project(metadata_test C)

set(CMAKE_C_STANDARD 11)

add_executable(${PROJECT_NAME} main.c)

#FFmpeg的安装目录，msys2中安装的动态编译
set(FFMPEG_DIR "D:/msys64/home/young/ffmpeg-n4.4.1/build64_shared")
#set(FFMPEG_DIR "D:/msys64/home/young/ffmpeg-n4.4.1/build64")
#头文件搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC ${FFMPEG_DIR}/include)
#库文件按搜索路径
target_link_directories(${PROJECT_NAME} PUBLIC ${FFMPEG_DIR}/lib)

target_link_libraries(${PROJECT_NAME} PUBLIC
        avcodec avdevice avfilter avformat avutil postproc swresample swscale
        )