package com.cnting.livepush.livepush;

import android.content.Context;

import com.cnting.livepush.record.BaseVideoRecorder;
import com.cnting.livepush.record.XYEncodecRender;

import javax.microedition.khronos.egl.EGLContext;

public class DefaultVideoPush extends BaseVideoPush {

    private XYEncodecRender xyEncodecRender;

    public DefaultVideoPush(Context context, EGLContext eglContext,int textureId) {
        super(context,eglContext,textureId);
        xyEncodecRender = new XYEncodecRender(context, textureId);
        setRender(xyEncodecRender);

    }
}
