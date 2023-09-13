package com.cnting.livepush.activity

import android.content.res.Configuration
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.cnting.livepush.databinding.ActivityVideoRecordBinding
import com.cnting.livepush.record.BaseVideoRecorder.OnMediaInfoListener
import com.cnting.livepush.record.DefaultVideoRecorder
import java.io.File

/**
 * Created by cnting on 2023/9/12
 *
 */
class VideoRecordActivity : AppCompatActivity() {
    private lateinit var binding: ActivityVideoRecordBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityVideoRecordBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val audioPath = File(getExternalFilesDir(null), "Utakata.mp3")
        val savePath = File(getExternalFilesDir(null), "recorder.mp4")
        var isRecord = false

        val videoRecorder = DefaultVideoRecorder(this, binding.cameraView.textureId)
        videoRecorder.setOnMediaInfoListener(object : OnMediaInfoListener {
            override fun onMediaTime(times: Long) {
                runOnUiThread {
                    if (isRecord) {
                        binding.startRecordBtn.text = "录制中:$times"
                    } else {
                        binding.startRecordBtn.text = "开始录制"
                    }
                }
            }

            override fun onInitCodecComplete() {
                videoRecorder.startRecord()
            }
        })


        binding.startRecordBtn.setOnClickListener {
            if (!isRecord) {
                //共享textureId和eglContext
                videoRecorder.initCodec(
                    binding.cameraView.eglContext,
                    audioPath.absolutePath,
                    savePath.absolutePath,
                    1080 / 2 / 2,
                    1920 / 2 / 2,
                )

            } else {
                videoRecorder.stopRecord()
            }
            isRecord = !isRecord
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        binding.cameraView.onDestory()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        binding.cameraView.previewAngle(this)
    }
}