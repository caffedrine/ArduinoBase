#ifndef DisplaySegShift4Digits_h
#define DisplaySegShift4Digits_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Use defines to link the hardware configurations to the correct numbers

class DisplaySegShift4Digits
{
	public:
		DisplaySegShift4Digits(int latch, int clock, int data, int digitPins[4]);
		void setNumber(int numberFrom0to9999);

	private:
		int getDecVal(int);
		void DisplayADigit(int , byte);
		
		int latchPin;
		int dataPin;
		int clockPin;
		int digitPins[4];
		
		byte digitVal[10];
};

#endif
/// END ///
