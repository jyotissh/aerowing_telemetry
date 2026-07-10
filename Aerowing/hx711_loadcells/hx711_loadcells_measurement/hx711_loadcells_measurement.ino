#include "HX711.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define calibration_factor -12384.0
#define calibration_factor_2 -12384.0
#define calibration_factor_3 -12384.0


#define DOUT  4
#define DOUT2 2
#define DOUT3 7

#define CLK   18

HX711 scale;
HX711 scale2;
HX711 scale3;

WiFiClient client;

const char* SSID = "CMF";
const char* PASS = "Hippo123$";
const char* HOST_IP = "10.151.156.21"; // Default IP of Windows Hotspot (Check via 'ipconfig') use localip
const int HOST_PORT = 80;

void WiFi_init();
void connectToServer();

const char* FB_URL = "https://aerowing-telemetry-dashboard-default-rtdb.asia-southeast1.firebasedatabase.app";

void setup() {

  Serial.begin(921600);  
  WiFi_init();
  if (!client.connected()) {
    connectToServer();
    delay(1000); // Don't spam reconnects
  }
  delay(500);           

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare(20);        

  scale2.begin(DOUT2,CLK);
  scale2.set_scale(calibration_factor_2);
  scale2.tare(20);        

  scale3.begin(DOUT3,CLK);
  scale3.set_scale(calibration_factor_3);
  scale3.tare(20);    

}    
void loop() {

  if (!client.connected()) {
    connectToServer();
    delay(1000);
  }

  Serial.println(String(scale.get_units(2), 3)+","+String(scale2.get_units(2), 3)+","+String(scale3.get_units(2), 3));
  //Serial.println("Hello");
  String packet = "Load," + String(scale.get_units(2), 3)+","+String(scale2.get_units(2), 3)+","+String(scale3.get_units(2), 3);


  size_t bytesSent = client.println(packet);
  if (bytesSent == 0) {
    Serial.println("Send failed");
  }

  client.flush();

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 3000) {
    lastCheck = millis();
    checkCommands();
  }


  delay(50);

}

void sendAck(String type, int cell, float factor) {
  HTTPClient http;
  http.begin(String(FB_URL) + "/ack.json");
  http.addHeader("Content-Type","application/json");
  String body = "{\"type\":\"" + type + "\",\"cell\":" + cell + ",\"factor\":" + String(factor,4) + "}";
  http.PUT(body);
  http.end();
}

void checkCommands() {
  HTTPClient http;

  // Tare
  for (int cell = 1; cell <= 3; cell++) {
    String url = String(FB_URL) + "/commands/tare.json";
    http.begin(url);
    if (http.GET() == 200) {
      DynamicJsonDocument doc(256);
      deserializeJson(doc, http.getString());
      if (doc["trigger"] == true && doc["cell"] == cell) {
        if (cell == 1) scale.tare(20);
        if (cell == 2) scale2.tare(20);
        if (cell == 3) scale3.tare(20);
        http.end();
        http.begin(url);
        http.addHeader("Content-Type","application/json");
        http.PATCH("{\"trigger\":false}");
        sendAck("tare", cell, 0);
      }
    }
    http.end();
  }
    // Calibrate
  String calUrl = String(FB_URL) + "/commands/calibrate.json";
  http.begin(calUrl);
  if (http.GET() == 200) {
    DynamicJsonDocument doc(256);
    deserializeJson(doc, http.getString());
    if (doc["trigger"] == true) {
      int cell = doc["cell"];
      float weight = doc["weight"];
      float raw = 0;
      if (cell == 1) raw = scale.get_value(20);
      if (cell == 2) raw = scale2.get_value(20);
      if (cell == 3) raw = scale3.get_value(20);
      float factor = raw / weight;
      if (cell == 1) scale.set_scale(factor);
      if (cell == 2) scale2.set_scale(factor);
      if (cell == 3) scale3.set_scale(factor);
      http.end();
      http.begin(calUrl);
      http.addHeader("Content-Type","application/json");
      http.PATCH("{\"trigger\":false}");
      sendAck("calibrate", cell, factor);
    }
  }
  http.end();
}

void WiFi_init(){
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nWiFi Connected.");
  Serial.println(WiFi.localIP());
}

void connectToServer() {
  Serial.println("Attempting connection to " + String(HOST_IP) + ":" + String(HOST_PORT));
  if (client.connect(HOST_IP, HOST_PORT)) {
    Serial.println("Connected to Server!");
    // Handshake: Tell the server who we are
    //client.println(DEVICE_ID); 
  }
  else{
    Serial.println("Connection FAILED");
  }
}