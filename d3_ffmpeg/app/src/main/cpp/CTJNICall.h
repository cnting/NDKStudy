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
    jobject jAudioTrackObj;
    jmethodID jAudioTrackWriteMid;
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jmethodID jPlayerErrorMid;
    jobject jPlayerObj;

    void callPlayerError(ThreadMode threadMode, int code, char *msg);

public:
    CTJNICall(JavaVM *javaVm, JNIEnv *jniEnv, jobject jPlayerObj);

    ~CTJNICall();

    void callAudioTrackWrite(jbyteArray audioData, int offsetInBytes, int sizeInBytes) const;

private:
    void initCreateAudioTrack();
};


#endif //D3_FFMPEG_CTJNICALL_H
