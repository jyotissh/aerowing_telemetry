#include "HX711.h"

#define DOUT  4
#define CLK   5

HX711 scale;

float calibration_factor = -12900; 

void setup() {

  Serial.begin(115200);
  delay(500);

  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare();

  long zero_factor = scale.read_average();
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);
}

void loop() {

  scale.set_scale(calibration_factor);

  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 3);
  Serial.print(" kg");               
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();
  delay(500);

  if (Serial.available()) {
    char temp = Serial.read();
    if (temp == '+' || temp == 'a')
      calibration_factor += 500;
    else if (temp == '-' || temp == 'z')
      calibration_factor -= 500;     
    else if (temp == 't')         // press t to re-tare anytime
      scale.tare(10);
  }
  
}