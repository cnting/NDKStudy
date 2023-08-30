package com.cnting.opencv

import android.graphics.BitmapFactory
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.isVisible
import com.cnting.opencv.databinding.ActivityFaceDetectionBinding
import com.permissionx.guolindev.PermissionX
import org.opencv.android.BaseLoaderCallback
import org.opencv.android.CameraBridgeViewBase
import org.opencv.android.CameraBridgeViewBase.CAMERA_ID_FRONT
import org.opencv.android.LoaderCallbackInterface
import org.opencv.android.OpenCVLoader
import org.opencv.core.Mat
import org.opencv.core.Scalar
import org.opencv.objdetect.CascadeClassifier
import java.io.File
import java.io.FileOutputStream
import java.io.IOException


/**
 * Created by cnting on 2023/8/21
 * 人脸识别
 */
class FaceDetectionActivity : AppCompatActivity() {
    private lateinit var binding: ActivityFaceDetectionBinding
    private val faceDetection = FaceDetection()
    private lateinit var mCascadeFile: File

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityFaceDetectionBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.faceBtn.setOnClickListener {
            openCameraView(false)
            faceDetection()
        }
        binding.cameraBtn.setOnClickListener {
            openCameraView(true)
        }
        binding.cameraView.setCvCameraViewListener(cameraListener)
        binding.cameraView.setCameraIndex(CAMERA_ID_FRONT)

        PermissionX.init(this).permissions(android.Manifest.permission.CAMERA)
            .request { allGranted, _, _ ->
                if (allGranted) {
                    binding.cameraView.setCameraPermissionGranted()
                }
            }
    }

    private fun openCameraView(open: Boolean) {
        binding.image.isVisible = !open
        binding.cameraView.isVisible = open
        if (open) {
            binding.cameraView.enableView()
        } else {
            binding.cameraView.disableView()
        }
    }

    override fun onResume() {
        super.onResume()
        if (!OpenCVLoader.initDebug()) {
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, loaderCallback)
        } else {
            loaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS)
        }
    }

    override fun onPause() {
        super.onPause()
        binding.cameraView.disableView()
    }

    private val loaderCallback = object : BaseLoaderCallback(this) {
        override fun onManagerConnected(status: Int) {
            if (status != LoaderCallbackInterface.SUCCESS) {
                super.onManagerConnected(status)
                return
            } else {
                copyCascadeFile()
                faceDetection.loadCascade(mCascadeFile.absolutePath)
            }
        }

    }

    private fun faceDetection() {
        val bitmap = BitmapFactory.decodeResource(resources, R.drawable.b)
        faceDetection.faceDetectionBitmap(bitmap)
        binding.image.setImageBitmap(bitmap)
    }


    private fun copyCascadeFile() {
        try {
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

    private val cameraListener = object : CameraBridgeViewBase.CvCameraViewListener {
        override fun onCameraViewStarted(width: Int, height: Int) {
        }

        override fun onCameraViewStopped() {
        }

        /**
         * Camera方向问题 https://www.cnblogs.com/qq2523984508/p/10512396.html
         */
        override fun onCameraFrame(inputFrame: Mat): Mat {
            faceDetection.faceDetection(inputFrame.nativeObj);
            return inputFrame

        }

    }
}