#include <math.h>
#include <stdlib.h>

#define RT0   10000.0f
#define B     3977.0f
#define R     10000.0f
#define VCC   5000.0f  // mV
#define ADC_RES 4095.0f

const float T0 = 298.15f;  // 25°C in Kelvin

void setup() {
  Serial.begin(9600);
}

void loop() {
  //float rawADC = analogRead(6);
  float VRT = analogReadMilliVolts(6);  // overcounts voltage
  float VR  = VCC - VRT;                 

  if (VR == 0.0f) {
    Serial.println("Error: VR=0, check wiring");
    delay(500);
    return;
  }

  float RT      = (VRT / VR) * R;  // resistance of thermistor
  float lRatio = log(RT / RT0);  
  float tempK   = 1.0f / ((lRatio / B) + (1.0f / T0));
  float tempC   = tempK - 273.15f;

  Serial.print("Temperature: ");
  Serial.print(tempC);         Serial.print(" C\t");
  Serial.print(tempK);         Serial.print(" K\t");
  Serial.print(tempC * 1.8f + 32.0f); Serial.println(" F");
  
  /*
  Serial.print(VRT);
  Serial.print(" ; ");
  Serial.print(VR);
  Serial.print(" ; ");
  Serial.print(RT);
  Serial.print(" ; ");
  Serial.print(RT0);
  Serial.print(" ; ");
  Serial.print(tempK);
  Serial.print(" ; ");
  Serial.println(tempC);
  */
  

  delay(500);
}