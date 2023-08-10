# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("ffmpeg")

#引入头文件，以这个配置的目录为基准
include_directories(includes
        ffmpeg)

add_library( # Sets the name of the library.
        ffmpeg

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp

        ffmpeg/cmdutils.c
        ffmpeg/ffmpeg.c
        ffmpeg/ffmpeg_filter.c
        ffmpeg/ffmpeg_opt.c
        )

# 添加 ffmpeg 的 avcodec、swresample、avutil 模块
set(distribution_DIR ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libavcodec.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libswresample.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libavutil.so)
# 添加 ffmpeg 的 avcodec、swresample、avutil 模块 end======

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)


target_link_libraries( # Specifies the target library.
        ffmpeg
        avcodec
        swresample
        avutil
        ${log-lib})