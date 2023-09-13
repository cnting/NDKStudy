package com.cnting.livepush.record;

import android.util.Log;

import com.ywl5320.wlmedia.enums.WlComplete;
import com.ywl5320.wlmedia.listener.WlOnMediaInfoListener;

/**
 * Created by cnting on 2023/9/13
 */
public class SimpleWlOnMediaInfoListener implements WlOnMediaInfoListener {
    @Override
    public void onPrepared() {

    }

    @Override
    public void onError(int i, String s) {
        Log.e("===>",s);
    }

    @Override
    public void onComplete(WlComplete wlComplete, String s) {

    }

    @Override
    public void onTimeInfo(double v, double v1) {

    }

    @Override
    public void onSeekFinish() {

    }

    @Override
    public void onLoopPlay(int i) {

    }

    @Override
    public void onLoad(boolean b) {

    }

    @Override
    public byte[] decryptBuffer(byte[] bytes) {
        return new byte[0];
    }

    @Override
    public byte[] readBuffer(int i) {
        return new byte[0];
    }

    @Override
    public void onPause(boolean b) {

    }
}
