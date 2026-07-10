#include "HX711.h"
#define DOUT  2
#define CLK  18

HX711 scale;   // declared globally, no arguments

float calibration_factor = -12384;

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.begin(DOUT, CLK);   // pins set here instead of constructor
  scale.set_scale();
  scale.tare();             // Reset the scale to 0

  long zero_factor = scale.read_average();
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);
}

void loop() {
  scale.set_scale(calibration_factor);
  Serial.print("Reading: ");
  float weight_kg = scale.get_units() * 0.453592;
  Serial.print("Reading: ");
  Serial.print(weight_kg, 3);
  Serial.print(" kg");
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  if (Serial.available()) {
    char temp = Serial.read();
    if (temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if (temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }
}