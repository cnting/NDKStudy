//
// Created by cnting on 2023/9/8.
//

#ifndef D95_VIDEOPLAYER_CTVIDEO_H
#define D95_VIDEOPLAYER_CTVIDEO_H


#include "CTMedia.h"
#include "android/native_window.h"
#include "android/native_window_jni.h"
#include "CTAudio.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"

};

class CTVideo : public CTMedia {
public:
    SwsContext *pSwsContext = NULL;
    uint8_t *pFrameBuffer = NULL;
    int frameSize;
    AVFrame *pRgbaFrame = NULL;
    jobject surface = NULL;
    CTAudio *pAudio;
    double delayTime;
    /**
     * 默认情况下最合适的延迟时间 1秒钟24帧
     */
    double defaultDelayTime = 0.04;
public:
    CTVideo(int audioStreamIndex, CTJNICall *pJniCall,
            CTPlayerStatus *pPlayStatus,CTAudio *pAudio);

    ~CTVideo();

    void play();

    void setSurface(jobject surface);

    void privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    void release();

/**
 * 视频同步音频，计算一帧要休眠的时间
 * @param pFrame 当前视频帧
 * @return 休眠时间，单位秒
 */
    double getFrameSleepTime(AVFrame *pFrame);
};


#endif //D95_VIDEOPLAYER_CTVIDEO_H
