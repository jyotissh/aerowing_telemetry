#include <ESP8266WiFi.h>

const char* ssid = "CMF";
const char* pass = "Hippo123$";

WiFiServer server(80);
WiFiClient clients[7];

String clientBuf[7];

// Track which client slot is the motor sub-server (port 8080 origin)
// We broadcast Serial-received lines to ALL clients so the motor sub-server
// picks up throttle commands forwarded from bridge.py.

String serialBuf = "";
unsigned long lastClientStatus = 0;

void setup() {
  Serial.begin(921600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
    Serial.println();
  }

  server.begin();
  Serial.println("Server IP: " + WiFi.localIP().toString());
}

void loop() {
  // ── Accept new clients into empty slots ──────────────────────────────────
  WiFiClient newClient = server.available();
  if (newClient) {
    bool placed = false;
    for (int i = 0; i < 7; i++) {
      if (!clients[i].connected()) {
        clients[i] = newClient;
        clients[i].setTimeout(150);
        Serial.println("Client " + String(i) + " connected");
        placed = true;
        break;
      }
    }
    if (!placed) {
      newClient.stop();  // no free slot
    }
  }
  if (millis() - lastClientStatus >= 500) {
    lastClientStatus = millis();
    String status = "CLIENTS";
    for (int i = 0; i < 7; i++) {
      status += ",";
      status += clients[i].connected() ? "1" : "0";
    }
    Serial.println(status);
  }
  // ── Read from each TCP client → forward to Serial (uplink) ───────────────
  // add alongside the other globals
  String clientBuf[7];

  // ── Read from each TCP client → forward to Serial (uplink) ───────────────

  for (int i = 0; i < 7; i++) {
    if (clients[i].connected()) {
      while (clients[i].available()) {
        char c = (char)clients[i].read();
        if (c == '\n') {
          clientBuf[i].trim();
          if (clientBuf[i].length() > 0) {
            Serial.println(clientBuf[i]);
          }
          clientBuf[i] = "";
        } else {
          clientBuf[i] += c;
        }
      }
    }
  }
    // ── Read from Serial → broadcast to all TCP clients (downlink) ───────────
    // bridge.py writes throttle commands as "THROTTLE,m1,m2,m3,m4\n"
    // which we relay to every connected client (motor sub-server picks it up).
    while (Serial.available()) {
      char c = (char)Serial.read();
      if (c == '\n') {
        serialBuf.trim();
        if (serialBuf.startsWith("THROTTLE,") && serialBuf.length() > 0) {
          String payload = serialBuf + "\n";
          for (int i = 0; i < 7; i++) {
            if (clients[i].connected()) {
              clients[i].print(payload);
            }
          }
        }
        serialBuf = "";
      } else {
        serialBuf += c;
      }
    }
  }
