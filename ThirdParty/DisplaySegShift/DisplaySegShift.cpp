#include "DisplaySegShift.h"

DisplaySegShift::DisplaySegShift(int latch, int clock, int data)
{
	latchPin = latch;
	dataPin = data;
	clockPin = clock;
}

int DisplaySegShift::getDecVal(int number)
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

void DisplaySegShift::oneDigitSetNumber(int number)
{
	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, MSBFIRST, getDecVal(number));  
    digitalWrite(latchPin, HIGH);
}