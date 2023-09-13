package com.cnting.livepush.activity

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.cnting.livepush.databinding.ActivityLivepushBinding
import com.cnting.livepush.databinding.ActivityMainBinding
import com.cnting.livepush.livepush.ConnectListener
import com.cnting.livepush.livepush.LivePush

class LivePushActivity : AppCompatActivity() {

    private lateinit var binding: ActivityLivepushBinding
    private lateinit var livePush: LivePush

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityLivepushBinding.inflate(layoutInflater)
        setContentView(binding.root)

        livePush = LivePush("rtmp://192.168.1.5:1935/myapp/room")
        livePush.initConnect()
        livePush.connectListener = object : ConnectListener {
            override fun connectError(code: Int, msg: String) {
                Log.e("===>", "connect error:$msg")
            }

            override fun connectSuccess() {
                Log.d("===>", "connectSuccess")
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        livePush.stop()
    }
}