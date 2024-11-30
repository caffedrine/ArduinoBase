#include "DisplaySegShift4Digits.h"

byte digitVal[10] =      //seven segment digits in bits
{
	B00111111, //0
	B00000110, //1
	B01011011, //2
	B01001111, //3
	B01100110, //4
	B01101101, //5
	B01111101, //6
	B00000111, //7
	B01111111, //8
	B01101111  //9
};

DisplaySegShift4Digits::DisplaySegShift4Digits(int latch, int clock, int data, int *digPins)
{
	latchPin = latch;
	dataPin = data;
	clockPin = clock;
	digitPins = digPins;
	
	for (int i = 0; i<4; i++)
		pinMode(digitPins[i], OUTPUT);

	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
}

int DisplaySegShift4Digits::getDecVal(int number)
{
	if(number == 0) return 63;
	else if(number == 1) return 6;
	else if(number == 2) return 91;
	else if(number == 3) return 79;
	else if(number == 4) return 102;
	else if(number == 5) return 109;
	else if(number == 6) return 125;
	else if(number == 7) return 7;
	else if(number == 8) return 127;
	else if(number == 9) return 111;
	else return 0;
}

void DisplaySegShift4Digits::setNumber(int number)
{
	int d1, d2, d3, d4;
	
	if (number > 9999)
		number = 9999;
	if (number < 0)
		number = 0;

	d1 = (int)(number / 1000);    // Get thousands
	number = number - (d1 * 1000);
	d2 = (int)(number / 100);     // Get hundreds
	number = number - (d2 * 100);
	d3 = (int)(number / 10);      // Get tens
	d4 = number - (d3 * 10);        // Get units

	DisplayADigit(digitPins[0], byte(digitVal[d1]));  // Show thousands
	DisplayADigit(digitPins[1], byte(digitVal[d2]));  // Show hundreds
	DisplayADigit(digitPins[2], byte(digitVal[d3]));  // Show tens
	DisplayADigit(digitPins[3], byte(digitVal[d4]));  // Show units
}

void DisplaySegShift4Digits::DisplayADigit(int dispno, byte digit2disp)
{

	digitalWrite(latchPin, LOW);

	for (int i = 0; i<4; i++)
		digitalWrite(digitPins[i], LOW);

	// shift the bits out:
	shiftOut(dataPin, clockPin, MSBFIRST, digit2disp);

	digitalWrite(latchPin, HIGH);  // Set latch high to set segments.

	digitalWrite(dispno, HIGH);  // Drive one cathode low to turn on display.

	delay(4);  // Wait a bit for POV

}