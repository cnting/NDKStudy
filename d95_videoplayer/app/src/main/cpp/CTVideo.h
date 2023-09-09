//
// Created by cnting on 2023/9/8.
//

#ifndef D95_VIDEOPLAYER_CTVIDEO_H
#define D95_VIDEOPLAYER_CTVIDEO_H


#include "CTMedia.h"
#include "android/native_window.h"
#include "android/native_window_jni.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"

};

class CTVideo : public CTMedia {
public:
    SwsContext *pSwsContext = NULL;
    uint8_t *pFrameBuffer = NULL ;
    int frameSize;
    AVFrame *pRgbaFrame = NULL;
    jobject surface = NULL;
public:
    CTVideo(int audioStreamIndex, CTJNICall *pJniCall,
            CTPlayerStatus *pPlayStatus);
    ~CTVideo();

    void play();

    void setSurface(jobject surface);

    void privateAnalysisStream(ThreadMode threadMode, AVFormatContext* pFormatContext);

    void release();
};


#endif //D95_VIDEOPLAYER_CTVIDEO_H
