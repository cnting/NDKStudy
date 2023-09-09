package com.cnting.videoplayer.media

import android.content.Context
import android.graphics.PixelFormat
import android.util.AttributeSet
import android.util.Log
import android.view.SurfaceView
import com.cnting.videoplayer.media.listener.MediaErrorListener
import com.cnting.videoplayer.media.listener.MediaPreparedListener

/**
 * Created by cnting on 2023/9/8
 *
 */
class CTVideoView(context: Context?, attrs: AttributeSet?) : SurfaceView(context, attrs),
    MediaPreparedListener, MediaErrorListener {
    private val player = CTPlayer().also {
        it.preparedListener = this@CTVideoView
        it.errorListener = this@CTVideoView
    }

    init {
        //设置显示的像素格式
        holder.setFormat(PixelFormat.RGBA_8888)
    }

    fun play(url: String) {
        stop()
        player.setDataSource(url)
        player.prepare()
    }

    fun stop() {
        player.stop()
    }

    override fun onError(code: Int, msg: String) {
    }

    override fun onPrepared() {
        Log.d("===>","当前线程:${Thread.currentThread().name}")
        player.setSurface(holder.surface);
        player.play()
    }

}