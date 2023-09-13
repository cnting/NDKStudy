#指定最低版本
cmake_minimum_required(VERSION 3.22.1)

project("livepush")

#引入头文件
include_directories(${PROJECT_SOURCE_DIR}/librtmp)

#指定源文件的目录
aux_source_directory(${CMAKE_SOURCE_DIR} SRC_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/librtmp RTMP_LIST)

#不使用openssl
#CMAKE_C_FLAGS存放的内容会传给C编译器
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DNO_CRYPTO")

add_library(
        livepush

        SHARED

        ${SRC_LIST}
        ${RTMP_LIST}

)

find_library(
        log-lib
        log)


#为native-lib添加编译链接库
target_link_libraries(livepush -landroid OpenSLES ${log-lib} )