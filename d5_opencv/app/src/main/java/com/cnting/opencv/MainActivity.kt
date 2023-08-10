package com.cnting.opencv

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

        val bitmap = BitmapFactory.decodeResource(resources, R.drawable.b)
        binding.image.setImageBitmap(bitmap)
        binding.faceBtn.setOnClickListener { faceDetection(bitmap) }

        copyCascadeFile()
        faceDetection.loadCascade(mCascadeFile.absolutePath)

    }

    private fun faceDetection(bitmap: Bitmap) {
        faceDetection.faceDetectionSaveInfo(bitmap)
        binding.image.setImageBitmap(bitmap)
    }

    private fun copyCascadeFile() {
        try {
            // load cascade file from application resources
            val `is` = resources.openRawResource(R.raw.lbpcascade_frontalface)
            val cascadeDir = getDir("cascade", MODE_PRIVATE)
            mCascadeFile = File(cascadeDir, "lbpcascade_frontalface.xml")
            if (mCascadeFile.exists()) {
                return
            }
            val os = FileOutputStream(mCascadeFile)
            val buffer = ByteArray(4096)
            var bytesRead: Int
            while (`is`.read(buffer).also { bytesRead = it } != -1) {
                os.write(buffer, 0, bytesRead)
            }
            `is`.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }
}