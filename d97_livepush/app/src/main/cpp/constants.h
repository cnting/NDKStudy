//
// Created by cnting on 2023/9/6.
//

#ifndef D3_FFMPEG_CONSTANTS_H
#define D3_FFMPEG_CONSTANTS_H

#include <android/log.h>

#define TAG "tag"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

//-------播放错误码 start--------
#define INIT_RTMP_CONNECT_ERROR_CODE  -0x10
#define INIT_RTMP_CONNECT_STREAM_ERROR_CODE  -0x11

//-------播放错误码 end--------

#endif //D3_FFMPEG_CONSTANTS_H
