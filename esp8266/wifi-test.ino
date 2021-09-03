//wifi libs
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
//display libs
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//display consts
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//wifi station info
const char* ssid     = "ssid";
const char* password = "pass";
//static ip info
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
//webserver
ESP8266WebServer server(80);
//ultra sonic sensors pins
const int trigPinT1 = 12;
const int echoPinT1 = 14;
const int trigPinT2 = 15;
const int echoPinT2 = 13;
//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701
//duration of ultra sonic wave travel
long duration;

float distanceCmT1;
int levelT1 = 0;
float distanceCmT2;
int levelT2 = 0;

char lcdText[30];

void setup() {
  Serial.begin(9600);
  delay(10);

  startWifi();

  startWebServer();

  setupUltraSonicSensors();

  setupDisplay();
}

void loop(void) {
  getUltraSonicSensorData(1);
  getUltraSonicSensorData(2);
  server.handleClient();
  delay(1000);
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
  //Serial.print("Connecting to ");
  //Serial.print(ssid);
  //Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  //Serial.println("Connection established!");
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

void setupUltraSonicSensors() {
  pinMode(trigPinT1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPinT1, INPUT); // Sets the echoPin as an Input
  pinMode(trigPinT2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPinT2, INPUT); // Sets the echoPin as an Input
}

//if tanker = 1 -> tanker 1 , if tanker = 2 -> tanker 2, duh!
void getUltraSonicSensorData(int tanker) {
  int trigPin;
  int echoPin;
  float distance = 0;
  int level = 0;
  if (tanker == 1) {
    trigPin = trigPinT1;
    echoPin = echoPinT1;
  }
  else {
    trigPin = trigPinT2;
    echoPin = echoPinT2;
  }
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distance = duration * SOUND_VELOCITY / 2;

  level = map(distance, 0, 170, 100, 0);

  if (tanker == 1) {
    levelT1 = level;
    distanceCmT1 = distance;
  }
  else {
    levelT2 = level;
    distanceCmT2 = distance;
  }

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
