/*Example program for the Dorji DSPC01 module
The module is working fine on 5V!
*/

#include "DSPC01.h"

int delaytime=200; 
/*this is the delay time between requesting the value 
and reading the value. Should be 200ms according to datasheet 
but 100ms works fine for me.*/

DSPC01 dspc01;
void setup()
{
  Serial.begin(115200);
  dspc01.begin(A5,A4); //plug in any 2 pins where you connect (SCK SDA) to. Does not have to be the I2C bus.
  Serial.println("Compass calibration: Rotate twice horizontally, 360 deg in 3 seconds"); 
  dspc01.calibrate_compass(); //seems to have to be done once after powering up
  delay(6000);
}



void loop()
{
Serial.print("Pressure [Pa] ");
dspc01.request_pressure();
delay(delaytime);
Serial.print(dspc01.pressure());

Serial.print(" Altitude [m] ");
dspc01.request_altitude();
delay(delaytime);
Serial.print(dspc01.altitude()/10.0);


Serial.print(" Temperature [deg C] ");
dspc01.request_temperature();
delay(delaytime);
Serial.print(dspc01.temperature()/10.0);


Serial.print(" Compass [deg] ");
dspc01.request_compass();
delay(delaytime);
Serial.println(dspc01.compass());
}
