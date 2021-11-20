# WaterLevel
## Preface
This project aims to find a tanker water level using [**NodeMCU**](https://www.nodemcu.com/index_en.html) with [**ESP8266**](https://www.espressif.com/en/products/socs/esp8266) core and `HC-SR04P` Ultrasonic sensor and broadcasting it using a simple HTTP server inside NodeMCU then data can be read using a browser, android app, another NodeMCU, or anything that can create an HTTP request with `GET` method.

## NodeMCU ESP8266 (LoLin NodeMCU V3)
Unlike generic `ESP8266` models, this model offers a micro-USB connection using a `CH340G` (USB to serial chip) naturally soldered to `ESP8266`. after installing `CH340G` drivers and `ESP8266` drivers for Arduino IDE, this chip can be programmed with Arduino compatible codes with Arduino IDE.
### Specs
- `80-160MHZ` `RISC` microprocessor
- `~128KB` of RAM
- `802.11` support: `b/g/n`
- `TCP/IP` protocol stack
- ...


<img src="/images/nodemcu.jpg" alt="nodemcu" width="300"/>

### NodeMCU pins breakout
<img src="/images/nodemcupins.png" alt="nodemcupins" width="500"/>

## HC-SR04P Ultrasonic Sensor
This model is the second generation of **`HC-SR04`** sensor, unlike the first model which only can operate at **`5V DC`**, this new version can work with **`3.3V DC`** too so it's perfectly safe to wire the output of this sensor directly to ESP8266 GPIO pins which operate at `0V~3.3V` logic levels without using any sort of voltage divider.
### Specs
- Power Supply: `3.3V~5V DC`
- Effective Angle: `<15 degrees`
- Ranging Distance: `2cm-400cm`
- Connector Pins: `4-pins header (vcc, gnd, echo, trigger)`

<img src="/images/HCSR04P.jpg" alt="HCSR04P" width="200"/>

## How it works?
[TL;DR Full Arduino/C++ code here](https://github.com/yamin8000/WaterLevel/blob/main/esp8266/wifi-test.ino)

ESP8266 wifi stack offers two types, connection mode, and access point mode. in this project, we used the connection mode so ESP8266 automatically tries to connect to the station with the given SSID and password and in this project, we used a static IP for `ESP8266` so we can find it more simply in the local area network.

Two sensors are wired to chip through GPIO pins, pin 12 or D6 is the trigger pin for sensor #1 and `pin 14` or `D5` is the echo pin for sensor #1 and `pin 15` `(D8)` and `13` `(D7)` for sensor #2. each second data from both sensors is read and mapped to the new range because each sensor send distance between itself and water (something between `0cm` to `170cm`) so this range is better to be mapped to a more readable range like `100-0`) so if the distance is 160 then the water level is almost 90 percent.

Using a simple HTTP server with port `80` data is sent for any client that tries to get data from the root of the server (http://192.168.1.100). Raw Data is like this:

```
T1 70% 53 T2 85% 28
```

Which stands for TankerName TankerPercent TankerSensorDistance.

### Broweser Client
<img src="/images/rawdata.png" alt="rawdata" width="500"/>

### Android Client
There is a simple Android App written in Kotlin that uses OkHttp for getting sensor data and shows it in simple text and battery-like view.

<img src="/images/android.png" alt="android" width="400"/>
