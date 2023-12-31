package com.cnting.videoplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.Surface
import android.widget.TextView
import com.cnting.videoplayer.databinding.ActivityMainBinding
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
//        val file = File(getExternalFilesDir(null), "Butterfly-209.mp4")
        val file = File(getExternalFilesDir(null), "juren-30s.mp4")

        binding.playBtn.setOnClickListener {
            binding.videoView.play(file.absolutePath)
//            binding.videoView.play("rtmp://localhost:1935/abcs/room")
        }
    }

}