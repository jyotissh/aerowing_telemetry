#include <WiFi.h>
const char* ssid = "CMF";
const char* pass = "Hippo123$";
WiFiServer server(80);

WiFiClient clients[5];

void setup() {
  Serial.begin(921600);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  server.begin();
  Serial.println("Server IP: " + WiFi.localIP().toString());
}

void loop() {
  // Accept new clients into empty slots
  WiFiClient newClient = server.available();
  if (newClient) {
    for (int i = 0; i < 5; i++) {
      if (!clients[i].connected()) {
        clients[i] = newClient;
        Serial.println("Client " + String(i) + " connected");
        break;
      }
    }
  }

  // Read from each connected client
  for (int i = 0; i < 5; i++) {
    if (clients[i].connected() && clients[i].available()) {
      String line = clients[i].readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        Serial.println(line);
      }
    }
  }
}