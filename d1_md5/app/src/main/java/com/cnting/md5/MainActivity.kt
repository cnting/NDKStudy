package com.cnting.md5

import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.widget.TextView
import com.cnting.md5.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val signature = SignatureUtils.signatureParams("userName=aaaa&&pwd=bbbbb")
        binding.sampleText.text = signature

        //获取签名
        val packageInfo = packageManager.getPackageInfo(packageName, PackageManager.GET_SIGNATURES);
        val signatures = packageInfo.signatures;
        Log.e("===>", signatures[0].toCharsString())
    }


}