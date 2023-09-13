package com.cnting.livepush.record;


import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.view.Surface;

import com.cnting.livepush.egl.EglHelper;
import com.cnting.livepush.egl.XYEGLSurfaceView;
import com.cnting.livepush.util.LogUtil;
import com.ywl5320.wlmedia.WlMedia;
import com.ywl5320.wlmedia.enums.WlPlayModel;
import com.ywl5320.wlmedia.listener.WlOnPcmDataListener;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.util.concurrent.CyclicBarrier;

import javax.microedition.khronos.egl.EGLContext;


public abstract class BaseVideoRecorder {

    private Surface surface;
    private EGLContext eglContext;
    private int width;
    private int height;
    private MediaCodec videoCodec;
    private MediaCodec audioCodec;
    private long audioPts = 0;
    private int sampleRate;
    private MediaMuxer mediaMuxer;
    private final WlMedia wlMedia;

    private final CyclicBarrier startCb = new CyclicBarrier(2, new Runnable() {
        @Override
        public void run() {
            if (mediaMuxer != null) {
                mediaMuxer.start();
            }
        }
    });
    private final CyclicBarrier destroyCb = new CyclicBarrier(2, new Runnable() {
        @Override
        public void run() {
            if (mediaMuxer != null) {
                mediaMuxer.stop();
                mediaMuxer.release();
                mediaMuxer = null;
            }
        }
    });

    private XYEGLMediaThread xyeglMediaThread;
    private VideoCodecThread videoCodecThread;
    private AudioCodecThread audioCodecThread;

    private OnMediaInfoListener onMediaInfoListener;

    //控制手动刷新还是自动刷新
    public final static int RENDERMODE_WHEN_DIRTY = 0;
    public final static int RENDERMODE_CONTINUOUSLY = 1;
    private int mRenderMode = RENDERMODE_CONTINUOUSLY;

    private XYEGLSurfaceView.XYGLRender xyGLRender;

    public BaseVideoRecorder(Context context) {
        wlMedia = new WlMedia();
        wlMedia.setCallBackPcmData(true);
        wlMedia.setPlayModel(WlPlayModel.PLAYMODEL_ONLY_AUDIO);
    }

    public void setRender(XYEGLSurfaceView.XYGLRender xyGLRender) {
        this.xyGLRender = xyGLRender;
    }

    public void setRenderMode(int mRenderMode) {
        this.mRenderMode = mRenderMode;
    }


    public void initCodec(EGLContext eglContext, String audioPath, String savePath, int width, int height) {
        this.width = width;
        this.height = height;
        this.eglContext = eglContext;

        wlMedia.setSource(audioPath);
        wlMedia.prepared();

        wlMedia.setOnMediaInfoListener(new SimpleWlOnMediaInfoListener() {
            @Override
            public void onPrepared() {
                super.onPrepared();
                wlMedia.start();
            }
        });
        wlMedia.setOnPcmDataListener(new WlOnPcmDataListener() {
            @Override
            public void onPcmInfo(int bit, int channel, int sampleRate) {
                initMediaCodec(savePath, width, height, sampleRate, channel);
            }

            @Override
            public void onPcmData(int size, byte[] data, double db) {
                putPCMData(data, size);
            }
        });

    }

    public void setOnMediaInfoListener(OnMediaInfoListener onMediaInfoListener) {
        this.onMediaInfoListener = onMediaInfoListener;
    }

    public void startRecord() {
        if (surface != null && eglContext != null) {

            audioPts = 0;

            xyeglMediaThread = new XYEGLMediaThread(new WeakReference<BaseVideoRecorder>(this));
            videoCodecThread = new VideoCodecThread(new WeakReference<BaseVideoRecorder>(this));
            audioCodecThread = new AudioCodecThread(new WeakReference<BaseVideoRecorder>(this));
            xyeglMediaThread.isCreate = true;
            xyeglMediaThread.isChange = true;
            xyeglMediaThread.start();
            videoCodecThread.start();
            audioCodecThread.start();
        }
    }

    public void stopRecord() {
        if (xyeglMediaThread != null && videoCodecThread != null && audioCodecThread != null) {
            videoCodecThread.exit();
            audioCodecThread.exit();
            xyeglMediaThread.onDestory();
            videoCodecThread = null;
            xyeglMediaThread = null;
            audioCodecThread = null;
            wlMedia.stop();
        }
    }


    /**
     * 封装器
     */
    private void initMediaCodec(String savePath, int width, int height, int sampleRate, int channelcount) {
        try {
            mediaMuxer = new MediaMuxer(savePath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
            initVideoCodec(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
            initAudioCodec(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate, channelcount);
            if (onMediaInfoListener != null) {
                onMediaInfoListener.onInitCodecComplete();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * 初始化视频编码器
     */
    private void initVideoCodec(String mimeType, int width, int height) {
        MediaFormat videoFormat;
        try {
            videoFormat = MediaFormat.createVideoFormat(mimeType, width, height);
            videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);//Surface
            videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, width * height * 4);//码率
            videoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30);//帧率
            videoFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);//I帧 关键帧的间隔  设置为1秒

            //编码
            videoCodec = MediaCodec.createEncoderByType(mimeType);
            videoCodec.configure(videoFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            surface = videoCodec.createInputSurface();
            videoCodec.start();

        } catch (IOException e) {
            LogUtil.e(e.getMessage());
            e.printStackTrace();
            videoCodec = null;
        }

    }

    /**
     * 初始化音频编码器
     *
     * @param mimeType     格式
     * @param sampleRate   采样率
     * @param channelCount 声道数
     */
    private void initAudioCodec(String mimeType, int sampleRate, int channelCount) {
        MediaFormat audioFormat;
        try {
            this.sampleRate = sampleRate;
            audioFormat = MediaFormat.createAudioFormat(mimeType, sampleRate, channelCount);
            audioFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);//比特率
            audioFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);//等级
            audioFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, sampleRate * channelCount * 2);

            audioCodec = MediaCodec.createEncoderByType(mimeType);
            audioCodec.configure(audioFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            audioCodec.start();
        } catch (IOException e) {
            e.printStackTrace();
            audioCodec = null;
        }
    }

    /**
     * EGL线程 渲染线程
     */
    static class XYEGLMediaThread extends Thread {
        private WeakReference<BaseVideoRecorder> encoder;
        private EglHelper eglHelper;
        private Object object;

        private boolean isExit = false;
        //记录是否创建
        private boolean isCreate = false;
        private boolean isChange = false;
        private boolean isStart = false;

        public XYEGLMediaThread(WeakReference<BaseVideoRecorder> encoder) {
            this.encoder = encoder;
        }

        @Override
        public void run() {
            super.run();
            isExit = false;
            isStart = false;
            object = new Object();
            eglHelper = new EglHelper();

            eglHelper.initEgl(encoder.get().surface, encoder.get().eglContext);
            while (true) {
                if (isExit) {
                    release();
                    break;
                }
                if (isStart) {
                    if (encoder.get().mRenderMode == RENDERMODE_WHEN_DIRTY) {
                        //手动刷新
                        synchronized (object) {
                            try {
                                object.wait();
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    } else if (encoder.get().mRenderMode == RENDERMODE_CONTINUOUSLY) {
                        try {
                            Thread.sleep(1000 / 60);//每秒60帧
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    } else {
                        throw new RuntimeException("mRenderMode is wrong value");
                    }
                }

                onCreate();
                onChange(encoder.get().width, encoder.get().height);
                onDraw();
                isStart = true;
            }
        }


        private void onCreate() {
            if (isCreate && encoder.get().xyGLRender != null) {
                isCreate = false;
                encoder.get().xyGLRender.onSurfaceCreated();
            }
        }

        private void onChange(int width, int height) {
            if (isChange && encoder.get().xyGLRender != null) {
                isChange = false;
                encoder.get().xyGLRender.onSurfaceChanged(width, height);
            }
        }

        private void onDraw() {
            if (encoder.get().xyGLRender != null && eglHelper != null) {
                encoder.get().xyGLRender.onDrawFrame();
                //必须调用两次 才能显示
                if (!isStart) {
                    encoder.get().xyGLRender.onDrawFrame();
                }
                eglHelper.swapBuffers();
            }
        }


        private void requestRunder() {
            if (object != null) {
                synchronized (object) {
                    object.notifyAll();//解除
                }
            }
        }

        public void onDestory() {
            isExit = true;
            requestRunder();

        }

        public void release() {
            if (eglHelper != null) {
                eglHelper.destoryEgl();
                eglHelper = null;
                object = null;
                encoder = null;
            }
        }

    }

    /**
     * 视频录制编码的线程
     */
    static class VideoCodecThread extends Thread {

        private final WeakReference<BaseVideoRecorder> encoder;
        private volatile boolean isExit;
        private MediaCodec videoCodec;
        private final MediaCodec.BufferInfo videoBufferInfo;
        private final MediaMuxer mediaMuxer;
        private final CyclicBarrier startCb;
        private final CyclicBarrier destroyCb;

        private int videoTrackIndex = -1;//视频轨道

        private long pts;

        public VideoCodecThread(WeakReference<BaseVideoRecorder> encoder) {
            this.encoder = encoder;
            videoCodec = encoder.get().videoCodec;
            mediaMuxer = encoder.get().mediaMuxer;
            startCb = encoder.get().startCb;
            destroyCb = encoder.get().destroyCb;
            videoBufferInfo = new MediaCodec.BufferInfo();
            videoTrackIndex = -1;
        }

        @Override
        public void run() {
            super.run();
            try {
                isExit = false;

                videoTrackIndex = -1;
                pts = 0;
                while (true) {
                    if (isExit) {
                        return;
                    }
                    //得到队列中可用的输出的索引
                    int outputBufferIndex = videoCodec.dequeueOutputBuffer(videoBufferInfo, 0);
                    if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                        //添加轨道
                        videoTrackIndex = mediaMuxer.addTrack(videoCodec.getOutputFormat());
                        startCb.await();
                    } else {
                        long lastMediaTime = 0;
                        while (outputBufferIndex >= 0) {

                            ByteBuffer outputBuffer = videoCodec.getOutputBuffers()[outputBufferIndex];
                            outputBuffer.position(videoBufferInfo.offset);
                            outputBuffer.limit(videoBufferInfo.offset + videoBufferInfo.size);
                            if (pts == 0) {
                                pts = videoBufferInfo.presentationTimeUs;
                            }
                            videoBufferInfo.presentationTimeUs = videoBufferInfo.presentationTimeUs - pts;//实现pts递增
                            mediaMuxer.writeSampleData(videoTrackIndex, outputBuffer, videoBufferInfo);

                            //时间回调
                            if (encoder.get().onMediaInfoListener != null) {
                                long time = videoBufferInfo.presentationTimeUs / 1000000;
                                if (time != lastMediaTime) {
                                    encoder.get().onMediaInfoListener.onMediaTime(time);
                                    lastMediaTime = time;
                                }
                            }

                            //编码完了释放
                            videoCodec.releaseOutputBuffer(outputBufferIndex, false);
                            outputBufferIndex = videoCodec.dequeueOutputBuffer(videoBufferInfo, 0);
                        }
                    }

                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                onDestory();
            }

        }

        private void onDestory() {
            videoCodec.stop();
            videoCodec.release();
            videoCodec = null;

            try {
                destroyCb.await();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }


        public void exit() {
            isExit = true;
        }
    }

    /**
     * put PCM 的数据
     *
     * @param buffer
     * @param size
     */
    public void putPCMData(byte[] buffer, int size) {
        if (audioCodecThread != null && !audioCodecThread.isExit && buffer != null && size > 0) {
            int inputBufferIndex = audioCodec.dequeueInputBuffer(0);
            if (inputBufferIndex >= 0) {
                ByteBuffer byteBuffer = audioCodec.getInputBuffers()[inputBufferIndex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                long pts = getAudioPts(size, sampleRate);
                audioCodec.queueInputBuffer(inputBufferIndex, 0, size, pts, 0);
            }
        }
    }

    /**
     * 音频编码的线程
     */
    static class AudioCodecThread extends Thread {

        private volatile boolean isExit;

        private MediaCodec audioCodec;
        private final MediaCodec.BufferInfo audioBufferInfo;

        private final MediaMuxer mediaMuxer;
        private final CyclicBarrier startCb;
        private final CyclicBarrier destroyCb;

        //轨道
        private int audioTrackIndex = -1;

        public AudioCodecThread(WeakReference<BaseVideoRecorder> encoder) {
            audioCodec = encoder.get().audioCodec;
            mediaMuxer = encoder.get().mediaMuxer;
            startCb = encoder.get().startCb;
            destroyCb = encoder.get().destroyCb;
            audioBufferInfo = new MediaCodec.BufferInfo();
            audioTrackIndex = -1;
        }

        @Override
        public void run() {
            super.run();
            try {
                long pts = 0;
                isExit = false;

                while (true) {
                    if (isExit) {
                        return;
                    }

                    int outputBufferIndex = audioCodec.dequeueOutputBuffer(audioBufferInfo, 0);
                    if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                        if (mediaMuxer != null) {
                            audioTrackIndex = mediaMuxer.addTrack(audioCodec.getOutputFormat());
                            startCb.await();
                        }
                    } else {

                        while (outputBufferIndex >= 0) {
                            ByteBuffer outputBuffer = audioCodec.getOutputBuffers()[outputBufferIndex];
                            outputBuffer.position(audioBufferInfo.offset);
                            outputBuffer.limit(audioBufferInfo.offset + audioBufferInfo.size);
                            //outputBuffer 编码
                            if (pts == 0) {
                                pts = audioBufferInfo.presentationTimeUs;
                            }
                            audioBufferInfo.presentationTimeUs = audioBufferInfo.presentationTimeUs - pts;//实现pts递增
                            mediaMuxer.writeSampleData(audioTrackIndex, outputBuffer, audioBufferInfo);


                            audioCodec.releaseOutputBuffer(outputBufferIndex, false);
                            outputBufferIndex = audioCodec.dequeueOutputBuffer(audioBufferInfo, 0);
                        }
                    }


                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                onDestory();
            }

        }

        private void onDestory() {
            //回收资源
            audioCodec.stop();
            audioCodec.release();
            audioCodec = null;

            try {
                destroyCb.await();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public void exit() {
            isExit = true;
        }
    }

    public interface OnMediaInfoListener {
        void onMediaTime(long times);

        void onInitCodecComplete();
    }

    private long getAudioPts(int size, int sampleRate) {
        audioPts = audioPts + (long) ((1.0 * size) / (sampleRate * 2 * 2) * 1000000.0);
        return audioPts;
    }
}














