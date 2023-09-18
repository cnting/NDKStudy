package com.cnting.livepush.livepush;


import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaRecorder;
import android.view.Surface;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import com.cnting.livepush.egl.EglHelper;
import com.cnting.livepush.egl.XYEGLSurfaceView;
import com.cnting.livepush.util.LogUtil;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.util.concurrent.CyclicBarrier;

import javax.microedition.khronos.egl.EGLContext;


public abstract class BaseVideoPush {

    private Surface surface;
    private Context context;
    private int textureId;
    private EGLContext eglContext;
    private int width;
    private int height;
    private MediaCodec videoCodec;
    private MediaCodec audioCodec;
    private long audioPts = 0;
    private int sampleRate = 11400;
    private int channel = 2;
    private LivePush livePush;
    private LivePush.ConnectListener connectListener;

    private XYEGLMediaThread xyeglMediaThread;
    private VideoEncoderThread videoEncoderThread;
    private AudioEncoderThread audioEncoderThread;
    private AudioRecordThread audioRecordThread;

    private XYEGLSurfaceView.XYGLRender xyGLRender;

    public BaseVideoPush(Context context, EGLContext eglContext, int textureId) {
        this.context = context;
        this.eglContext = eglContext;
        this.textureId = textureId;
    }

    public void setRender(XYEGLSurfaceView.XYGLRender xyGLRender) {
        this.xyGLRender = xyGLRender;
    }

    public void setOnConnectionListener(LivePush.ConnectListener listener) {
        this.connectListener = listener;
    }

    public void initVideo(String liveUrl, int width, int height) {
        this.width = width;
        this.height = height;

        initVideoCodec(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
        initAudioCodec(MediaFormat.MIMETYPE_AUDIO_AAC, 11400, 2);

        livePush = new LivePush(liveUrl);
        livePush.setConnectListener(new LivePush.ConnectListener() {
            @Override
            public void connectError(int code, @NonNull String msg) {
                if (connectListener != null) {
                    connectListener.connectError(code, msg);
                }
            }

            @Override
            public void connectSuccess() {
                if (connectListener != null) {
                    connectListener.connectSuccess();
                }
                startThread();
            }
        });
    }

    private void startThread() {
        LogUtil.d("===>surface:" + surface + ",eglContext:" + eglContext);
//        if (surface != null && eglContext != null) {
        audioPts = 0;
        xyeglMediaThread = new XYEGLMediaThread(new WeakReference<BaseVideoPush>(this));
        videoEncoderThread = new VideoEncoderThread(new WeakReference<BaseVideoPush>(this));
        audioEncoderThread = new AudioEncoderThread(new WeakReference<BaseVideoPush>(this));
        audioRecordThread = new AudioRecordThread(new WeakReference<>(this));
        xyeglMediaThread.isCreate = true;
        xyeglMediaThread.isChange = true;
        xyeglMediaThread.start();
        videoEncoderThread.start();
        audioEncoderThread.start();
        audioRecordThread.start();
//        }
    }


    public void startPush() {
        livePush.initConnect();
    }

    public void stopPush() {
        if (videoEncoderThread != null) {
            videoEncoderThread.exit();
            videoEncoderThread = null;
        }
        if (audioEncoderThread != null) {
            audioEncoderThread.exit();
            audioEncoderThread = null;
        }
        if (xyeglMediaThread != null) {
            xyeglMediaThread.onDestory();
            xyeglMediaThread = null;
        }
        if (audioRecordThread != null) {
            audioRecordThread.onDestory();
            audioRecordThread = null;
        }
        if (livePush != null) {
            livePush.stop();
            livePush = null;
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
        private WeakReference<BaseVideoPush> encoder;
        private EglHelper eglHelper;
        private Object object;

        private boolean isExit = false;
        //记录是否创建
        private boolean isCreate = false;
        private boolean isChange = false;
        private boolean isStart = false;

        public XYEGLMediaThread(WeakReference<BaseVideoPush> encoder) {
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
                    try {
                        Thread.sleep(1000 / 60);//每秒60帧
                    } catch (InterruptedException e) {
                        e.printStackTrace();
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
    static class VideoEncoderThread extends Thread {

        private final WeakReference<BaseVideoPush> encoder;
        private volatile boolean isExit;
        private MediaCodec videoCodec;
        private final MediaCodec.BufferInfo videoBufferInfo;
        private long pts;
        private byte[] videoSps, videoPps;

        public VideoEncoderThread(WeakReference<BaseVideoPush> encoder) {
            this.encoder = encoder;
            videoCodec = encoder.get().videoCodec;
            videoBufferInfo = new MediaCodec.BufferInfo();
        }

        @Override
        public void run() {
            super.run();
            try {
                isExit = false;

                pts = 0;
                while (true) {
                    if (isExit) {
                        return;
                    }
                    //得到队列中可用的输出的索引
                    int outputBufferIndex = videoCodec.dequeueOutputBuffer(videoBufferInfo, 0);
                    if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                        //获取sps和pps
                        ByteBuffer byteBuffer = videoCodec.getOutputFormat().getByteBuffer("csd-0");
                        videoSps = new byte[byteBuffer.remaining()];
                        byteBuffer.get(videoSps, 0, videoSps.length);

                        byteBuffer = videoCodec.getOutputFormat().getByteBuffer("csd-1");
                        videoPps = new byte[byteBuffer.remaining()];
                        byteBuffer.get(videoPps, 0, videoPps.length);
                    } else {
                        while (outputBufferIndex >= 0) {

                            ByteBuffer outputBuffer = videoCodec.getOutputBuffers()[outputBufferIndex];
                            outputBuffer.position(videoBufferInfo.offset);
                            outputBuffer.limit(videoBufferInfo.offset + videoBufferInfo.size);
                            if (pts == 0) {
                                pts = videoBufferInfo.presentationTimeUs;
                            }
                            videoBufferInfo.presentationTimeUs = videoBufferInfo.presentationTimeUs - pts;//实现pts递增

                            //在关键帧前，先把sps和pps推到流媒体服务器
                            if (videoBufferInfo.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME) {
                                encoder.get().livePush.pushSpsPPs(videoSps, videoSps.length, videoPps, videoPps.length);
                            }
                            byte[] data = new byte[outputBuffer.remaining()];
                            outputBuffer.get(data, 0, data.length);
                            //推送一帧数据
                            encoder.get().livePush.pushVideo(data, data.length, videoBufferInfo.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME);

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
        if (audioEncoderThread != null && !audioEncoderThread.isExit && buffer != null && size > 0) {
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
    static class AudioEncoderThread extends Thread {

        private volatile boolean isExit;
        private WeakReference<BaseVideoPush> encoder;
        private MediaCodec audioCodec;
        private final MediaCodec.BufferInfo audioBufferInfo;


        public AudioEncoderThread(WeakReference<BaseVideoPush> encoder) {
            this.encoder = encoder;
            audioCodec = encoder.get().audioCodec;
            audioBufferInfo = new MediaCodec.BufferInfo();
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

                    while (outputBufferIndex >= 0) {
                        ByteBuffer outputBuffer = audioCodec.getOutputBuffers()[outputBufferIndex];
                        outputBuffer.position(audioBufferInfo.offset);
                        outputBuffer.limit(audioBufferInfo.offset + audioBufferInfo.size);
                        //outputBuffer 编码
                        if (pts == 0) {
                            pts = audioBufferInfo.presentationTimeUs;
                        }
                        audioBufferInfo.presentationTimeUs = audioBufferInfo.presentationTimeUs - pts;//实现pts递增

                        byte[] data = new byte[outputBuffer.remaining()];
                        outputBuffer.get(data, 0, data.length);
                        encoder.get().livePush.pushAudio(data, data.length);

                        audioCodec.releaseOutputBuffer(outputBufferIndex, false);
                        outputBufferIndex = audioCodec.dequeueOutputBuffer(audioBufferInfo, 0);
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
        }

        public void exit() {
            isExit = true;
        }
    }

    /**
     * 音频采集线程
     */
    static class AudioRecordThread extends Thread {

        private volatile boolean isExit;
        private AudioRecord audioRecord;
        //pcm数据
        private byte[] audioData;
        private final int minBufferSize;
        private final int sampleRate;
        private WeakReference<BaseVideoPush> encoder;

        public AudioRecordThread(WeakReference<BaseVideoPush> encoder) {
            this.encoder = encoder;
            sampleRate = encoder.get().sampleRate;
            minBufferSize = AudioRecord.getMinBufferSize(sampleRate,
                    AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT);
            if (ActivityCompat.checkSelfPermission(encoder.get().context, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
                    sampleRate,
                    AudioFormat.CHANNEL_IN_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT, minBufferSize);
            audioData = new byte[minBufferSize];
        }

        @Override
        public void run() {
            super.run();
            try {
                audioRecord.startRecording();
                isExit = false;

                while (true) {
                    if (isExit) {
                        return;
                    }

                    audioRecord.read(audioData, 0, minBufferSize);

                    encoder.get().putPCMData(audioData, minBufferSize);
                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                onDestory();
            }

        }

        private void onDestory() {
            exit();
            try {
                audioRecord.stop();
                audioRecord.release();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public void exit() {
            isExit = true;
        }
    }

    private long getAudioPts(int size, int sampleRate) {
        audioPts = audioPts + (long) ((1.0 * size) / (sampleRate * 2 * 2) * 1000000.0);
        return audioPts;
    }
}














