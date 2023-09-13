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
    jmethodID jConnectErrorMid;
    jmethodID jConnectSuccessMid;
    jobject jLiveObj;



public:
    CTJNICall(JavaVM *javaVm, JNIEnv *jniEnv, jobject jLiveObj);

    ~CTJNICall();

    void callConnectError(ThreadMode threadMode, int code, char *msg);
    void callConnectSuccess(ThreadMode threadMode);
};


#endif //D3_FFMPEG_CTJNICALL_H
