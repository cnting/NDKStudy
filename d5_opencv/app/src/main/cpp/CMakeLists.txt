cmake_minimum_required(VERSION 3.22.1)

project("opencv")

include_directories(include)

#编译头文件
file(GLOB my_source_path ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.c)
add_library( # Sets the name of the library.
        opencv

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp bitmap_utils.cpp)

#添加动态链接库
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libopencv_java4.so)


find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# 在ndk中查找jnigraphics库 取别名jnigraphics-lib
# jnigraphics包含图形操作的库
find_library(jnigraphics-lib jnigraphics)


target_link_libraries( # Specifies the target library.
        opencv
        lib_opencv
        ${jnigraphics-lib}
        ${log-lib})