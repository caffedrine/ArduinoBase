#ifndef DisplaySegShift_h
#define DisplaySegShift_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Use defines to link the hardware configurations to the correct numbers

 


class DisplaySegShift
{
	public:
		DisplaySegShift(int latch, int clock, int data);
		void oneDigitSetNumber(int numberFrom0to9);

	private:
		int getDecVal(int);
		
		int latchPin;
		int dataPin;
		int clockPin;
};

#endif
/// END ///
