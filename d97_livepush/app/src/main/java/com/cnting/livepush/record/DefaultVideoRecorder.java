package com.cnting.livepush.record;

import android.content.Context;

/**
 * @author liuml
 * @explain
 * @time 2018/12/11 16:41
 */
public class DefaultVideoRecorder extends BaseVideoRecorder {

    private XYEncodecRender xyEncodecRender;

    public DefaultVideoRecorder(Context context, int textureId) {
        super(context);
        xyEncodecRender = new XYEncodecRender(context, textureId);
        setRender(xyEncodecRender);
        setRenderMode(BaseVideoRecorder.RENDERMODE_CONTINUOUSLY);

    }
}
