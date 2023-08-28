package com.cnting.opencv

import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.cnting.opencv.databinding.ActivityMainBinding
import java.io.File
import java.io.FileOutputStream
import java.io.IOException


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val faceDetection = FaceDetection()
    private lateinit var mCascadeFile: File

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.faceBtn.setOnClickListener {
            startActivity(Intent(this, FactDetectionActivity::class.java))
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
    }

    companion object {
        init {
            System.loadLibrary("opencv")
        }
    }
}