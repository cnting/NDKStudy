package com.cnting.jni

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.cnting.jni.databinding.ActivityMainBinding
import java.util.UUID

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val name = "张三";

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        //jni获取属性并修改
        changeField()
        binding.sampleText.text = name

        changeStaticField()
        binding.sampleText1.text = "年龄：$age"

        callJavaMethod()
        callJavaStaticMethod()

        val point = createNativePoint()
        binding.sampleText2.text = "point:(${point.x},${point.y})"

        val arr = intArrayOf(3, 5, 7, 2, 4, 1)
        sortArray(arr)
        binding.sampleText3.text = arr.contentToString()

        localRef()

        saveGlobalRef("1234")
        binding.sampleText4.text = getGlobalRef()
        deleteGlobalRef()
        //删除后再次获取会报错
//        binding.sampleText4.text = getGlobalRef()

        staticLocalCache("aaa")
        Log.d("===>", "修改staticName:$staticName")
        staticLocalCache("bbb")
        Log.d("===>", "修改staticName:$staticName")
        staticLocalCache("ccc")
        Log.d("===>", "修改staticName:$staticName")

        try {
            exception()
        } catch (e: NoSuchFieldException) {
            e.printStackTrace()
        }

        arrayCopy()
    }

    private fun arrayCopy() {
        val persons = arrayOfNulls<Person>(10)
        (0 until 10).forEach {
            persons[it] = Person("ct", it + 1)
        }
        val newPerson = arrayOfNulls<Person>(10)

        arraycopy(persons, 0, newPerson, 0, persons.size)

        newPerson.forEach {
            Log.i("===>", "数组拷贝 第${it}个：${it?.name}")
        }
    }

    fun add(a: Int, b: Int): Int {
        return a + b;
    }

    external fun changeField()
    external fun changeStaticField()
    external fun callJavaMethod()
    external fun callJavaStaticMethod()
    external fun createNativePoint(): Point
    external fun sortArray(arr: IntArray)
    external fun localRef()
    external fun saveGlobalRef(str: String)
    external fun getGlobalRef(): String
    external fun deleteGlobalRef()
    external fun staticLocalCache(str: String)
    external fun initStaticCache()
    external fun arraycopy(
        src: Any?, srcPos: Int,
        dest: Any?, destPos: Int,
        length: Int
    )

    @Throws(NoSuchFieldException::class)
    external fun exception()


    companion object {
        init {
            System.loadLibrary("jni")

        }

        val age = 10

        @JvmStatic
        var staticName = ""

        @JvmStatic
        fun getUUID(): String {
            return UUID.randomUUID().toString()
        }
    }
}