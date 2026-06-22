#include <ESP8266WiFi.h>

const char* ssid = "CMF";
const char* pass = "Hippo123$";

WiFiServer server(80);
WiFiClient clients[5];

void setup() {
  Serial.begin(921600);

  WiFi.mode(WIFI_STA);          // avoid stale AP+STA mode on ESP8266
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  server.begin();
  Serial.println("Server IP: " + WiFi.localIP().toString());
}

void loop() {
  // Accept new clients into empty slots
  WiFiClient newClient = server.available();
  if (newClient) {
    bool placed = false;
    for (int i = 0; i < 5; i++) {
      if (!clients[i].connected()) {
        clients[i] = newClient;
        clients[i].setTimeout(200); // 200ms read timeout for this client
        Serial.println("Client " + String(i) + " connected");
        placed = true;
        break;
      }
    }
    if (!placed) {
      newClient.stop();  // no free slot, reject cleanly instead of letting it hang
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
