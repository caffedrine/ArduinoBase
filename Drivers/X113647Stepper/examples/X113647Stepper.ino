#include "Arduino.h"

#include "X113647Stepper.h"

X11Stepper motor(40, 41, 42, 43);


void setup()
{
	Serial.begin(9600);

	motor.SetDirection(X11Stepper::DIRECTION::FORWARD);

}


void loop()
{
	motor.StepNext();
	delay(5);
}
