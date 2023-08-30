package com.cnting.opencv

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.cnting.opencv.databinding.ActivityMainBinding
import java.io.File


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val faceDetection = FaceDetection()
    private lateinit var mCascadeFile: File

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.faceBtn.setOnClickListener {
            startActivity(Intent(this, FaceDetectionActivity::class.java))
        }
        binding.filterBtn.setOnClickListener {
            startActivity(Intent(this, FilterActivity::class.java))
        }
        binding.specialEffectBtn.setOnClickListener {
            startActivity(Intent(this,SpecialEffectActivity::class.java))
        }
        binding.transformBtn.setOnClickListener {
            startActivity(Intent(this,TransformActivity::class.java))
        }
        binding.detectBtn.setOnClickListener {
            startActivity(Intent(this,ObjDetectActivity::class.java))
        }
    }

    companion object {
        init {
            System.loadLibrary("opencv")
        }
    }
}