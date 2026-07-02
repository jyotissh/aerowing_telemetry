#include <Servo.h>

const int THROTTLE_MIN = 1100;
const int THROTTLE_MAX = 1940;
const int ARM_PULSE    = 1100;

Servo motors[4];
const int motors_pins[4] = {5, 6, 7, 8};

String serialBuf = "";

void setup() {
  // Match the motor sub-server ESP8266 baud rate
  Serial.begin(115200);
  Servo_init();
  delay(3000);
  Serial.println("Armed. Waiting for THROTTLE,m1,m2,m3,m4 commands.");
}

void loop() {
  // Accumulate characters until newline
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      serialBuf.trim();
      if (serialBuf.startsWith("THROTTLE,")) {
        handleThrottle(serialBuf);
      }
      serialBuf = "";
    } else {
      serialBuf += c;
    }
  }
}

void handleThrottle(const String& line) {
  // Format: THROTTLE,1100,1200,1300,1400
  int vals[4] = {0, 0, 0, 0};
  int idx = 0;
  int start = line.indexOf(',') + 1;  // skip "THROTTLE,"

  for (int i = 0; i < 4 && start > 0; i++) {
    int end = line.indexOf(',', start);
    String token = (end == -1) ? line.substring(start) : line.substring(start, end);
    vals[i] = token.toInt();
    start = (end == -1) ? -1 : end + 1;
    idx++;
  }

  for (int i = 0; i < 4; i++) {
    if (vals[i] >= THROTTLE_MIN && vals[i] <= THROTTLE_MAX) {
      motors[i].writeMicroseconds(vals[i]);
    } else if (vals[i] != 0) {
      Serial.print("Motor ");
      Serial.print(i + 1);
      Serial.println(" value out of range, ignored.");
    }
  }

  Serial.print("M1:");  Serial.print(vals[0]);
  Serial.print(" M2:"); Serial.print(vals[1]);
  Serial.print(" M3:"); Serial.print(vals[2]);
  Serial.print(" M4:"); Serial.println(vals[3]);
}

void Servo_init() {
  for (int i = 0; i < 4; i++) {
    motors[i].attach(motors_pins[i]);
    motors[i].writeMicroseconds(ARM_PULSE);
  }
}
