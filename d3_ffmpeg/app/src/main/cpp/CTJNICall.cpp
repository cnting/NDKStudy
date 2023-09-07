//
// Created by cnting on 2023/9/7.
//

#include "CTJNICall.h"
#include "constants.h"

CTJNICall::CTJNICall(JavaVM *javaVm, JNIEnv *jniEnv, jobject jPlayerObj) {
    this->javaVM = javaVm;
    this->jniEnv = jniEnv;
    //全局引用可以跨线程
    this->jPlayerObj = jniEnv->NewGlobalRef(jPlayerObj);
    initCreateAudioTrack();

    jclass jPlayerClass = jniEnv->FindClass("com/cnting/ffmpeg/media/CTPlayer");
    jPlayerErrorMid = jniEnv->GetMethodID(jPlayerClass, "onError", "(ILjava/lang/String;)V");
}

CTJNICall::~CTJNICall() {
    jniEnv->DeleteGlobalRef(jPlayerObj);
}

void CTJNICall::initCreateAudioTrack() {
/**
 * public AudioTrack(int streamType, int sampleRateInHz, int channelConfig, int audioFormat,
            int bufferSizeInBytes, int mode)
 */
    //创建AudioTrack
    int streamType = 3;
    int sampleRateInHz = AUDIO_SAMPLE_RATE;
    int channelConfig = (0x4 | 0x8);
    int audioFormat = 2;
    int bufferSizeInBytes;
    int mode = 1;

    jclass jAudioTrackClass = jniEnv->FindClass("android/media/AudioTrack");
    jmethodID jAudioTrackMid = jniEnv->GetMethodID(jAudioTrackClass, "<init>", "(IIIIII)V");

    jmethodID getMinBufferSizeMid = jniEnv->GetStaticMethodID(jAudioTrackClass, "getMinBufferSize",
                                                              "(III)I");
    bufferSizeInBytes = jniEnv->CallStaticIntMethod(jAudioTrackClass, getMinBufferSizeMid,
                                                    sampleRateInHz, channelConfig, audioFormat);

    jAudioTrackObj = jniEnv->NewObject(jAudioTrackClass, jAudioTrackMid, streamType,
                                       sampleRateInHz, channelConfig,
                                       audioFormat, bufferSizeInBytes, mode);

    //调用play()
    jmethodID playMid = jniEnv->GetMethodID(jAudioTrackClass, "play", "()V");
    jniEnv->CallVoidMethod(jAudioTrackObj, playMid);

    jAudioTrackWriteMid = jniEnv->GetMethodID(jAudioTrackClass, "write", "([BII)I");
}

void
CTJNICall::callAudioTrackWrite(jbyteArray audioData, int offsetInBytes, int sizeInBytes) const {
    jniEnv->CallIntMethod(jAudioTrackObj, jAudioTrackWriteMid, audioData, offsetInBytes,
                          sizeInBytes);
}

void CTJNICall::callPlayerError(ThreadMode threadMode, int code, char *msg) {
    if (threadMode == THREAD_MAIN) {
        jstring str = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jPlayerObj, jPlayerErrorMid, code, str);
        jniEnv->DeleteLocalRef(str);
    } else {
        JNIEnv *env;
        javaVM->AttachCurrentThread(&env, 0);

        jstring str = env->NewStringUTF(msg);
        env->CallVoidMethod(jPlayerObj, jPlayerErrorMid, code, str);
        env->DeleteLocalRef(str);

        javaVM->DetachCurrentThread();
    }
}
