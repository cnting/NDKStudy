#include <jni.h>
#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

FILE *pcmFile;
void *pcmBuffer;

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    if (!feof(pcmFile)) {
        //读1秒数据
        fread(pcmBuffer, 1, 44100 * 2 * 2, pcmFile);
        //加入到队列
        (*bq)->Enqueue(bq, pcmBuffer, 44100 * 2 * 2);
    } else {
        fclose(pcmFile);
        free(pcmBuffer);
    }
}

void initCreateOpenSLES() {
    //1. 创建引擎接口对象
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;

    // create engine
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);

    // realize the engine
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);

    // get the engine interface, which is needed in order to create other objects
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);


    //2. 设置混音器
    SLObjectItf outputMixObject = NULL;
    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);

    // realize the output mix
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    // get the environmental reverb interface
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings =
            SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                     &outputMixEnvironmentalReverb);
    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
            outputMixEnvironmentalReverb, &reverbSettings);


    //3. 创建播放器
    SLObjectItf pPlayer = NULL;
    SLPlayItf bqPlayerPlay;
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource pAudioSrc = {&loc_bufq, &format_pcm};
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink pAudioSnk = {&loc_outmix, NULL};
    const SLInterfaceID pInterfaceIds[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME,
                                            SL_IID_PLAYBACKRATE,};
    const SLboolean pInterfaceRequired[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,};
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pPlayer, &pAudioSrc, &pAudioSnk,
                                       3, pInterfaceIds, pInterfaceRequired);
    // realize the player
    (*pPlayer)->Realize(pPlayer, SL_BOOLEAN_FALSE);

    // get the play interface
    (*pPlayer)->GetInterface(pPlayer, SL_IID_PLAY, &bqPlayerPlay);

    //4. 设置缓存队列和回调函数
    // get the buffer queue interface
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    (*pPlayer)->GetInterface(pPlayer, SL_IID_BUFFERQUEUE,
                             &bqPlayerBufferQueue);
    // register callback on the buffer queue
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);


    //5. 设置播放状态
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    //6. 调用回调函数
    bqPlayerCallback(bqPlayerBufferQueue, NULL);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_opensles_MainActivity_playPCM(JNIEnv *env, jobject thiz, jstring path) {
    const char *cpath = env->GetStringUTFChars(path, 0);
    pcmFile = fopen(cpath, "r");
    pcmBuffer = malloc(44100 * 2 * 2);
    initCreateOpenSLES();
    env->ReleaseStringUTFChars(path, cpath);
}