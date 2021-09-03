package com.github.yamin8000.waterlevel

import android.graphics.Color
import android.os.Bundle
import android.widget.Toast
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
    }
    
    private fun updateData() {
        mainScope.launch {
            while (true) {
                getDataFromSensorSever {
                    runOnUiThread {
                        if (it.isNotBlank()) updateUi(it)
                        else showNoDataError()
                    }
                }
                delay(5000)
            }
        }
    }
    
    private fun showNoDataError() {
        baseContext?.let {
            Toast.makeText(baseContext, "error", Toast.LENGTH_SHORT).show()
        }
    }
    
    private fun updateUi(data : String) {
        Logger.d(data)
        val dataSplit = data.split(' ')
        if (dataSplit.isNotEmpty()) {
            val firstTankerPercent = dataSplit[1]
            val secondTankerPercent = dataSplit[4]
            showTankerPercents(firstTankerPercent, secondTankerPercent)
            showTankerLevels(firstTankerPercent, secondTankerPercent)
        }
    }
    
    private fun showTankerPercents(firstTankerPercent : String, secondTankerPercent : String) {
        binding.firstTankerData.text = firstTankerPercent
        binding.secondTankerData.text = secondTankerPercent
    }
    
    private fun showTankerLevels(firstTankerPercent : String, secondTankerPercent : String) {
        val firstTankerLevel = (firstTankerPercent.trimEnd('%').toInt() / 20)
        val secondTankerLevel = (secondTankerPercent.trimEnd('%').toInt() / 20)
        
        val firstTankerLevelIndicators = listOf(binding.firstTanker1, binding.firstTanker2,
                                                binding.firstTanker3, binding.firstTanker4,
                                                binding.firstTanker5)
        
        val secondTankerLevelIndicators = listOf(binding.secondTanker1, binding.secondTanker2,
                                                 binding.secondTanker3, binding.secondTanker4,
                                                 binding.secondTanker5)
        
        val indicators = listOf("1", "2", "3", "4", "5")
        
        firstTankerLevelIndicators.forEach {
            if (it.tag in indicators.subList(0, firstTankerLevel)) it.setBackgroundColor(Color.BLUE)
            else it.setBackgroundColor(Color.BLACK)
        }
        secondTankerLevelIndicators.forEach {
            if (it.tag in indicators.subList(0, secondTankerLevel)) it.setBackgroundColor(Color.BLUE)
            else it.setBackgroundColor(Color.BLACK)
        }
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