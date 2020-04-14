#include <Arduino.h>
#include <TCS34725.h>
#include "DHT.h"
#include <stdio.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define ACTIVATED LOW
// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground
// set to false if using a common cathode LED
#define commonAnode true
// our RGB -> eye-recognized gamma color
byte gammatable[256];

My_TCS34725 tcs = My_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_60X);

bool ledEnabled=false;
int lightsMode=0;
float AnalogVoltage;
float mapped_value;
float AnalogValue;
const int ANALOGPIN = 34;
const float RESELUTION = 4095.0;
const float VCC = 3.3;
const char * ssid = "anuja1";
const char * password = "ar3k57u4";
String postData;
WiFiServer server(80);
String user_id = "1";
float pH;
void setup() {
  // put your setup code here, to run once:
   for (int i=0; i<256; i++) {
      float x = i;
      x /= 255;
      x = pow(x, 2.5);
      x *= 255;      
      if (commonAnode) {
        gammatable[i] = 255 - x;
      } else {
        gammatable[i] = x;      
      }
   }
   WiFi.begin(ssid, password);
   WiFi.mode(WIFI_STA);
   Serial.begin(9600);
   Serial.println("Delay for preparation: ");
   delay(10000);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   // Print local IP address and start web server
   Serial.println("");
   Serial.println("WiFi connected.");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
   server.begin();
   pinMode(ANALOGPIN, INPUT);
   dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t clear, red, green, blue;
  tcs.getRGBC(&red, &green, &blue, &clear);
  // Figure out some basic hex code for visualization
  uint32_t sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 255; g *= 255; b *= 255;
  String redHex,greenHex,blueHex, FullHex;
  redHex =  String((int)r, HEX);  
  greenHex =  String((int)g, HEX);  
  blueHex =  String((int)b, HEX);

  FullHex = redHex + greenHex + blueHex;

  Serial.println("Color In RGB Format");
  Serial.println("--------------------------------------");

  Serial.print("R: ");
  Serial.println(((int)r));

  Serial.print("G: ");
  Serial.println(((int)g));

  Serial.print("B: ");
  Serial.println(((int)b));
  delay(1000);
  Serial.println("Color in HEX format");
  Serial.println("-------------------------------------");
  Serial.print("#");
  Serial.println(FullHex);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  AnalogValue = analogRead(ANALOGPIN);
  printf("Analog value: %f \n", AnalogValue);
  mapped_value = map(AnalogValue, 1200, 4095, 260, 10);
  AnalogVoltage = (AnalogValue / RESELUTION) * VCC;
  printf("Mapped value: %f \n", mapped_value);
  printf("Analog voltage: %f \n", AnalogVoltage);

  pH = random(14);
  // put your main code here, to run repeatedly:
  HTTPClient http;
  http.begin("http://192.168.43.212:8084/FarmersAssistant/User/Sensor.jsp");
  postData = "color=" + FullHex + "&temp=" + String(f) + "&humidity=" + String(h) + "&moisture=" + mapped_value + "&pH=" + String(pH) + "&userid=" +user_id;
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);
  
  String payload = http.getString();    //Get the response payload
  Serial.println(httpCode);   //Print HTTP return code
  delay(5000);
}