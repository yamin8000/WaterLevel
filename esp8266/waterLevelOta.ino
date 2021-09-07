//wifi libs
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
//ota
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//display libs
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//sonar lib
#include <NewPing.h>
//display consts
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//wifi station info
const char* ssid     = "-";
const char* password = "-";
//static ip info
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
//webserver
ESP8266WebServer server(80);
//ultra sonic sensors pins and data
const int trigPinT1 = 12;
const int echoPinT1 = 14;
const int trigPinT2 = 15;
const int echoPinT2 = 13;
#define MAX_DISTANCE 400
NewPing firstTanker(trigPinT1, echoPinT1, MAX_DISTANCE);
NewPing secondTanker(trigPinT2, echoPinT2, MAX_DISTANCE);
float distanceCmT1;
int levelT1 = 0;
float distanceCmT2;
int levelT2 = 0;

char lcdText[30];

void setup() {
  Serial.begin(9600);
  delay(10);

  startWifi();

  setupOta();

  startWebServer();

  setupDisplay();
}

void loop(void) {
  ArduinoOTA.handle();
  getUltraSonicSensorData();
  server.handleClient();
  delay(1000);
}

void setupOta() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void setupDisplay() {
  //Address 0x3D for 128x64
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  } else Serial.println("display init ok");
  delay(2000);

  writeToDisplay("Hello there!");
}

void startWifi() {
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void startWebServer() {
  if (MDNS.begin("esp8266")) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/", sendUltraSonicSensorData);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void getUltraSonicSensorData() {
  distanceCmT1 = firstTanker.ping_cm();
  distanceCmT2 = secondTanker.ping_cm();

  levelT1 = map(distanceCmT1, 0, 170, 100, 0);
  levelT2 = map(distanceCmT2, 0, 170, 100, 0);
  

  sprintf(lcdText, "T1 %d%% %d T2 %d%% %d", levelT1, (int) distanceCmT1, levelT2, (int) distanceCmT2);
  writeToDisplay(lcdText);
}

void sendUltraSonicSensorData() {
  server.send(200, "text/plain", lcdText);
}

void writeToDisplay(char* text) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println(text);
  display.display();
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}
