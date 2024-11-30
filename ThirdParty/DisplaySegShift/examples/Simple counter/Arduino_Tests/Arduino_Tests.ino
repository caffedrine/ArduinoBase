#include <DisplaySegShift\DisplaySegShift.h>

//Pin connected to ST_CP of 74HC595
int latchPin = 12;
//Pin connected to SH_CP of 74HC595
int clockPin = 13;
////Pin connected to DS of 74HC595
int dataPin = 11;

DisplaySegShift display(latchPin, clockPin, dataPin);


void setup() {
	//set pins to output so you can control the shift register
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
}

void loop()
{
	int del = 1000;

	display.oneDigitSetNumber(0);
	delay(del);
	display.oneDigitSetNumber(1);
	delay(del);
	display.oneDigitSetNumber(2);
	delay(del);
	display.oneDigitSetNumber(3);
	delay(del);
	display.oneDigitSetNumber(4);
	delay(del);
	display.oneDigitSetNumber(5);
	delay(del);
	display.oneDigitSetNumber(6);
	delay(del);
	display.oneDigitSetNumber(7);
	delay(del);
	display.oneDigitSetNumber(8);
	delay(del);
	display.oneDigitSetNumber(9);
	delay(del);

}