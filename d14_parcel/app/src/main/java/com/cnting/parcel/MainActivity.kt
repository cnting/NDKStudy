package com.cnting.parcel

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.widget.TextView
import com.cnting.parcel.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val parcel = android.os.Parcel.obtain()
        parcel.writeInt(12)
        parcel.writeInt(24)

        parcel.setDataPosition(0)
        var num1 = parcel.readInt()
        var num2 = parcel.readInt()

        Log.e("===>", "num1:$num1,num2:$num2")

        val myParcel = Parcel()
        myParcel.writeInt(10)
        myParcel.writeInt(20)
        myParcel.setDataPosition(0)
        num1 = myParcel.readInt()
        num2 = myParcel.readInt()
        Log.e("===>", "自定义Parcel num1:$num1,num2:$num2")
    }

}