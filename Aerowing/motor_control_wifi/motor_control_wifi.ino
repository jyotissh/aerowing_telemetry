#include <ESP8266WiFi.h>

const char* ssid     = "CMF";
const char* password = "Hippo123$";

const char* mainServerIP = "10.159.73.148";
const uint16_t mainServerPort = 80;

WiFiClient client;

String tcpBuf    = "";
String serialBuf = "";

void setup() {
  // USB Serial (Debug)
  Serial.begin(115200);

  // UART1 TX (GPIO2) -> Nano Every RX1
  Serial1.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;

    if (attempts > 20) {
      Serial.println("\nWiFi failed! Restarting...");
      ESP.restart();
    }
  }

  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  // Maintain connection to main ESP8266 server
  if (!client.connected()) {
    Serial.print("Connecting to main server... ");

    if (client.connect(mainServerIP, mainServerPort)) {
      Serial.println("Connected");
    } else {
      Serial.println("Failed");
      delay(2000);
      return;
    }
  }

  // Optional: Forward anything from Nano to main server
  while (Serial1.available()) {
    char c = (char)Serial1.read();
    serialBuf += c;

    if (c == '\n') {
      client.print(serialBuf);
      serialBuf = "";
    }
  }

  // Receive throttle commands from main server
  while (client.available()) {
    char c = (char)client.read();
    tcpBuf += c;

    if (c == '\n') {
      tcpBuf.trim();

      if (tcpBuf.startsWith("THROTTLE,")) {
        String values = tcpBuf.substring(9);

        // Send to Nano Every
        Serial1.println(values);

        // Debug output on PC
        
        Serial.println(values);
      }

      tcpBuf = "";
    }
  }
}