
const int IR_PIN = 19;
int pulsesPerRev = 2;

volatile uint32_t pulse_count = 0;

void IRAM_ATTR isr() {
    pulse_count++;
}

void setup() {
    Serial.begin(115200);
    pinMode(IR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IR_PIN),isr, FALLING);
}

void loop(){
   
  static uint32_t last_time = 0;
  uint32_t now = millis();

  if (now - last_time >= 50) {
        noInterrupts();
        uint32_t count = pulse_count;
        Serial.printf("Pulse count: %d\n",count);
        //Serial.println("Pulse count: "+ String(count));
        pulse_count = 0;
        interrupts();

        float rpm = (count * 1200) / pulsesPerRev ; // 2 pulse per rev
        //Serial.println("RPM: "+ String(rpm));
        Serial.printf("RPM: %.2f\n", rpm);
        last_time = now;
  }


}