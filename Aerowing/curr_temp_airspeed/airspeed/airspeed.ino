//
//    FILE: MS4525DO_isConnected.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//     URL: https://github.com/RobTillaart/MS4525DO_RT


#include "MS4525DO.h"
#include <math.h>

#define rho 1.146f
#define one_psi 6894.76f

MS4525DO sensor(0x28);


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("MS4525DO_LIB_VERSION: ");
  Serial.println(MS4525DO_LIB_VERSION);
  Serial.println();

  Wire.begin(3,4);
  if (sensor.begin(100) == false)
  {
    Serial.print("Cannot find sensor:\t");
    Serial.print(sensor.getAddress());
    Serial.print(" - state: ");
    Serial.println(sensor.state());
    Serial.println("Check wires or try another address.");
    while(1);
  }
}


void loop()
{
  if (sensor.isConnected() == false)
  {
    Serial.println("cannot connect to sensor!");
  }
  else
  {
    delay(10);
    int state = sensor.read();
    if (state == MS4525DO_OK)
    {
      int16_t raw = sensor.rawPressureCount();
      // assume ±1 PSI range — change X if different
      float X = 1.0;
      float psi = abs(((float)raw - 8192.0) / 8192.0 * X);
      float pa = psi * 6894.76;
      float v = sqrt(2.0 * pa / rho);
      Serial.print("raw:\t"); Serial.println(raw);
      Serial.print("Pa:\t"); Serial.println(pa);
      Serial.print("m/s:\t"); Serial.println(v-4.5);
    
    }
    else
    {
      Serial.print("error: ");
      Serial.println(state);
    }
  }

  delay(1000);
}


//  -- END OF FILE --
