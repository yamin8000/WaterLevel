package com.github.yamin8000.waterlevel

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.github.yamin8000.waterlevel.databinding.ActivityMainBinding
import com.orhanobut.logger.AndroidLogAdapter
import com.orhanobut.logger.Logger
import com.orhanobut.logger.PrettyFormatStrategy
import kotlinx.coroutines.*
import okhttp3.*
import okio.IOException

class MainActivity : AppCompatActivity() {
    
    private val logTag = "<--> "
    private val url = "http://192.168.1.100"
    private val mainScope = CoroutineScope(Dispatchers.Main)
    
    private val binding : ActivityMainBinding by lazy(LazyThreadSafetyMode.NONE) {
        ActivityMainBinding.inflate(layoutInflater)
    }
    
    override fun onCreate(savedInstanceState : Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(binding.root)
        
        prepareLogger()
        
        updateData()
        binding.text.setOnClickListener { updateData() }
    }
    
    private fun updateData() {
        mainScope.launch {
            while (true) {
                getDataFromSensorSever { runOnUiThread { updateUi(it) } }
                delay(5000)
            }
        }
    }
    
    private fun updateUi(data : String) {
        binding.text.text = data
    }
    
    private val client = OkHttpClient()
    
    private fun getDataFromSensorSever(callback : (String) -> Unit) {
        val request = Request.Builder().url(url).build()
        
        client.newCall(request).enqueue(object : Callback {
            override fun onFailure(call : Call, e : IOException) {
                Logger.d("failed")
            }
            
            override fun onResponse(call : Call, response : Response) {
                val body = response.body?.string() ?: ""
                Logger.d(body)
                callback(body)
            }
        })
    }
    
    private fun prepareLogger() {
        Logger.addLogAdapter(AndroidLogAdapter(PrettyFormatStrategy.newBuilder().tag(logTag).build()))
        Logger.d("Application is Started!")
    }
    
    override fun onPause() {
        super.onPause()
        mainScope.cancel()
        Logger.d("pausing")
    }
}