  #include <math.h>
  #include <stdlib.h>
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <ArduinoJson.h>

  #include "MS4525DO.h"


  // macros 
  #define RT0   13.14f
  #define B     3977.0f
  #define R     11.0f
  #define VCC_GPIO6  3300.0f //mV
  #define rho 1.146f
  #define one_psi 6894.76f  // in P
  #define ADC_RES 4095.0f

  // user config
  const char* SSID = "CMF";
  const char* PASS = "Hippo123$";
  const char* HOST_IP = "10.204.206.148"; 
  const int HOST_PORT = 80;

  // change this for each module: MOTOR1, MOTOR2, MOTOR3, MOTOR4
  const char* DEVICE_ID = "MOTOR3"; 

  // WiFi objects
  WiFiClient client;

  // HTTP objects
  HTTPClient http;

  // airspeed sensor objects
  MS4525DO sensor(0x28);

  // WiFi functions
  void WiFi_init();
  void connectToServer();

  // sensor functions
  void current_sensor();
  void rpm_sensor();
  void temp_sensor();
  void airspeed_sensor();
  void current_sensor_init();
  void rpm_sensor_init();
  void temp_sensor_init();
  void airspeed_sensor_init();

  // extra functions
  void IRAM_ATTR isr_pulsecount();
  // void checkCurrentZero();

  /* global variables for sensor */

  // current sensor

  const int avgSamples = 10;
  const int analogInPin = 0;
  const float SUPPLY_VOLTAGE = 5000.0;
  float sensitivity = 13.2; // 50A module example
  int zeroCurrentVoltage = 1650; 
  static float current = 0;

  // rpm sensor

  const int IR_PIN = 19;
  static uint32_t pulse_count = 0;
  static float rpm;
  const int pulsesPerRev = 2;

  // temp sensor

  const float T0 = 298.15f;  // 25°C in Kelvin
  static float tempK;
  static float tempC;

  // airspeed sensor

  static float velocity;

  void setup() {
    // put your setup code here, to run once
    Serial.begin(921600);
    WiFi_init();
    if (!client.connected()) {
      connectToServer();
      delay(1000); // Don't spam reconnects
    }
    // sensor init
    current_sensor_init();
    airspeed_sensor_init();
    rpm_sensor_init();
    temp_sensor_init();

  }

  void loop() {

    // put your main code here, to run repeatedly

    if (!client.connected()) {
      connectToServer();
      delay(1000);
    }

    //noInterrupts();

    //interrupts();

    rpm_sensor();
    airspeed_sensor();
    current_sensor();
    temp_sensor();
    

    //Serial.println("Rpm: " + String(rpm) + " Current: " + String(current) + " Temperature: " + String(tempC)+ " Airspeed: " + String(velocity));

    
    static unsigned long lastSend = 0;
    if (millis() - lastSend >= 50) {
      lastSend = millis();

      String packet = String(DEVICE_ID) + "," + String(rpm, 1) + "," +
                      String(tempC, 1) + "," + String(current, 2) + "," +
                      String(velocity, 2);

      Serial.println(packet);
      size_t bytesSent = client.println(packet);
      if (bytesSent == 0) Serial.println("Send failed");
      client.flush();
    }
    



    // Firebase command poll every 3s 
    // static unsigned long lastCheck = 0;
    // if (millis() - lastCheck > 3000) {
    //   lastCheck = millis();
    //   checkCurrentZero();
    // }
    
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
      client.println(DEVICE_ID); 
    }
    else{
      Serial.println("Connection FAILED");
    }
  }

  const char* FB_URL = "https://aerowing-telemetry-dashboard-default-rtdb.asia-southeast1.firebasedatabase.app";
  /*
  void checkCurrentZero() {
    String url = String(FB_URL) + "/commands/current_zero.json";
    http.begin(url);
    if (http.GET() == 200) {
      DynamicJsonDocument doc(256);
      deserializeJson(doc, http.getString());
      if (doc["trigger"] == true) {
        if (doc["manual"] == true) {
          // User provided offset directly
          zeroCurrentVoltage = doc["offset"].as<int>();
          Serial.println("Manual offset set: " + String(zeroCurrentVoltage));
        } else {
          // Auto-sample
          long total = 0;
          for (int i = 0; i < 50; i++) total += analogReadMilliVolts(analogInPin);
          zeroCurrentVoltage = total / 50;
          Serial.println("Auto zero set: " + String(zeroCurrentVoltage));
        }
        http.end();
        http.begin(url);
        http.addHeader("Content-Type","application/json");
        http.PATCH("{\"trigger\":false}");
        HTTPClient h2;
        h2.begin(String(FB_URL) + "/ack.json");
        h2.addHeader("Content-Type","application/json");
        h2.PUT("{\"type\":\"current_zero\",\"offset\":" + String(zeroCurrentVoltage) + "}");
        h2.end();
      }
    }
    http.end();
  }
  */
  void IRAM_ATTR isr_pulsecount() {
      pulse_count++;
  }

  void current_sensor_init(void){
      long sum = 0;
      for (int i = 0; i < 10; i++) {
        sum += analogRead(analogInPin);
        delay(1);
      }
      zeroCurrentVoltage = (sum/10.0);
  }

  void rpm_sensor_init(void){
    
    pinMode(IR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IR_PIN),isr_pulsecount, FALLING);
    
  }

  void temp_sensor_init(void){
      /* None */
  }

  void airspeed_sensor_init(void){
    
    Wire.begin(3,4);
    
    if (sensor.begin(100) == false)
    {
      Serial.print("Cannot find sensor:\t");
      Serial.print(sensor.getAddress());
      Serial.print(" - state: ");
      Serial.println(sensor.state());
      Serial.println("Check wires or try another address.");
      while(1);
    }

    if (sensor.isConnected() == false)
    {
      Serial.println("cannot connect to sensor!");
      return;
    }


  }

  void current_sensor(void){

    long total_mV = 0;
    
    for(int i=0; i<avgSamples; i++) total_mV += analogReadMilliVolts(analogInPin);
    float avg_mV = (total_mV/avgSamples);
    // Serial.println("avg_mV (no-load): " + String(avg_mV));
    current = (avg_mV-zeroCurrentVoltage) / sensitivity;
    /*if (avg_mV > 100) {
      current = (avg_mV-zeroCurrentVoltage) / sensitivity;
    }*/
    // Serial.println("AvgCurrent: " + String(current));   
    
    
  }

  void rpm_sensor(void){
    static uint32_t last_time = 0;
    uint32_t now = millis();

    if ((now - last_time) >= 50) {
          noInterrupts();
          uint32_t count = pulse_count;
          pulse_count = 0;
          interrupts();

          rpm = (count * 1200) / pulsesPerRev ; // 2 pulse per rev
          // Serial.printf("RPM: %.2f\n", rpm);
          last_time = now;
    }
  }

  void temp_sensor(void){
    float VRT = analogReadMilliVolts(6);
    float VR  = VCC_GPIO6 - VRT;                 

    if (VR == 0.0f) {
      Serial.println("Error: VR=0, check wiring");
      delay(500);
      return;
    }

    float RT      = R * ((VCC_GPIO6 - VRT) / VRT);
    float lnRatio = log(RT / RT0);  
    tempK   = 1.0f / ((lnRatio / B) + (1.0f / T0));
    tempC   = tempK - 273.15f;

    /*
    Serial.print(RT);
    Serial.print("Temperature: ");
    Serial.print(tempC);         Serial.print(" C\t");
    Serial.print(tempK);         Serial.print(" K\t");
    Serial.print(tempC * 1.8f + 32.0f); Serial.println(" F");
    */
    
  }

  void airspeed_sensor(void){
      static unsigned long lastRead = 0;
      unsigned long now = millis();
      if (now - lastRead < 10) return;   // non-blocking equivalent of the old delay
      lastRead = now;

      int state = sensor.read();

      if (state == MS4525DO_OK)
      {
        int16_t raw = sensor.rawPressureCount();
        float X = 1.0;
        float psi = abs(((float)raw - 8192.0) / 8192.0 * X);
        float pa = psi * 6894.76;
        velocity = sqrt(2.0 * pa / rho);
      }
      /*
      else
      {
        Serial.print("error: ");
        Serial.println(state);
      }
      */
  }


