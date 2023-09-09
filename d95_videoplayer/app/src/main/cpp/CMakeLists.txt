#指定最低版本
cmake_minimum_required(VERSION 3.22.1)

project("videoplayer")

#引入头文件
include_directories(${PROJECT_SOURCE_DIR}/includes)



#指定共享库目录
link_directories(${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

#指定源文件的目录
aux_source_directory(${CMAKE_SOURCE_DIR} SRC_LIST)

add_library(
        videoplayer

        SHARED

        ${SRC_LIST}

        )

find_library(
        log-lib
        log)


# 添加 ffmpeg 的 avcodec、swresample、avutil 模块
set(distribution_DIR ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

#https://blog.csdn.net/LaineGates/article/details/108242803
#IMPORTED 设置直接导入已经生成的库，cmake不会给这类library添加编译规则
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libavcodec.so)

add_library(avdevice SHARED IMPORTED)
set_target_properties(avdevice PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libavdevice.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libavutil.so)

add_library(postproc SHARED IMPORTED)
set_target_properties(postproc PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libpostproc.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/libswscale.so)

# 添加 ffmpeg 的 avcodec、swresample、avutil 模块 end======

#为native-lib添加编译链接库
target_link_libraries(videoplayer avcodec avdevice avfilter avformat avutil postproc swresample swscale -landroid OpenSLES ${log-lib})