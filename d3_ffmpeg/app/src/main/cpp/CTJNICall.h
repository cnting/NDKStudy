//
// Created by cnting on 2023/9/7.
//

#ifndef D3_FFMPEG_CTJNICALL_H
#define D3_FFMPEG_CTJNICALL_H

#include <jni.h>

enum ThreadMode {
    THREAD_CHILD, THREAD_MAIN
};

class CTJNICall {
public:
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jmethodID jPlayerErrorMid;
    jmethodID jPlayerPreparedMid;
    jobject jPlayerObj;

    void callPlayerError(ThreadMode threadMode, int code, char *msg);

public:
    CTJNICall(JavaVM *javaVm, JNIEnv *jniEnv, jobject jPlayerObj);

    ~CTJNICall();

    void callPlayPrepared(ThreadMode threadMode);
};


#endif //D3_FFMPEG_CTJNICALL_H
