package com.cnting.livepush.activity

import android.content.res.Configuration
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.cnting.livepush.databinding.ActivityLivepushBinding
import com.cnting.livepush.livepush.DefaultVideoPush
import com.cnting.livepush.livepush.LivePush
import com.cnting.livepush.util.DisplayUtil
import com.cnting.livepush.util.LogUtil

class LivePushActivity : AppCompatActivity() {

    private lateinit var binding: ActivityLivepushBinding
    private lateinit var videoPush: DefaultVideoPush

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityLivepushBinding.inflate(layoutInflater)
        setContentView(binding.root)

        videoPush = DefaultVideoPush(
            this@LivePushActivity,
            binding.cameraView.eglContext,
            binding.cameraView.textureId
        )
        videoPush.initVideo(
            "rtmp://192.168.1.5:1935/myapp/room",
//            DisplayUtil.getScreenWidth(this@LivePushActivity),
//            DisplayUtil.getScreenHeight(this@LivePushActivity)
            720 / 2, 1280 / 2
        )
        videoPush.setOnConnectionListener(object : LivePush.ConnectListener {
            override fun connectError(code: Int, msg: String) {
                LogUtil.d("===>连接失败:$msg")
            }

            override fun connectSuccess() {
                LogUtil.d("===>连接成功")
            }

        })

        binding.livePushBtn.setOnClickListener {
            videoPush.startPush()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        videoPush.stopPush()
        binding.cameraView.onDestory()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        binding.cameraView.previewAngle(this)
    }
}