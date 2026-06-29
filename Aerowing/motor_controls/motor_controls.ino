#include <Servo.h>

const int THROTTLE_MIN = 1100;
const int THROTTLE_MAX = 1940;
const int ARM_PULSE = 1100;

Servo motors[4];
int motors_pins[4] ={5,6,7,8};

void Servo_init();

void setup() {
  Serial.begin(9600);
  Servo_init();
  delay(3000);
  Serial.println("Armed. Type a value 1100-1940 to set throttle.");
}

void loop() {
  if (Serial.available()) {
    int val1 = Serial.parseInt();
    int val2 = Serial.parseInt();
    int val3 = Serial.parseInt();
    int val4 = Serial.parseInt();

    int vals[4] = {val1,val2,val3,val4};

    while(Serial.available()){  // to remove spurious values
      Serial.read();
    }

    for(int i = 0;i<=3;i++){
      if( vals[i]>=THROTTLE_MIN && vals[i] <= THROTTLE_MAX ){
        motors[i].writeMicroseconds(vals[i]);
      }
      else if(vals[i] != 0){
        Serial.print("Motor ");
        Serial.print(i + 1);
        Serial.println(" out of range, ignored.");
      }
    }

    Serial.print("Motor1: ");
    Serial.print(val1);
    Serial.print("; Motor2: ");
    Serial.print(val2);
    Serial.print("; Motor3: ");
    Serial.print(val3);
    Serial.print("; Motor4: ");
    Serial.println(val4);  // only the last one uses println, to end the line  
  }
}

void Servo_init(void){
  
  for(int i = 0;i<=3;i++){
    motors[i].attach(motors_pins[i]);
    motors[i].writeMicroseconds(ARM_PULSE);
  }

}