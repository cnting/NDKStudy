package com.cnting.livepush.record;

import android.content.Context;

import javax.microedition.khronos.egl.EGLContext;

/**
 * @author liuml
 * @explain
 * @time 2018/12/11 16:41
 */
public class DefaultVideoRecorder extends BaseVideoRecorder {

    private XYEncodecRender xyEncodecRender;

    public DefaultVideoRecorder(Context context, EGLContext eglContext,int textureId) {
        super(context,eglContext,textureId);
        xyEncodecRender = new XYEncodecRender(context, textureId);
        setRender(xyEncodecRender);
        setRenderMode(BaseVideoRecorder.RENDERMODE_CONTINUOUSLY);

    }
}
