#include "Buzzer.h"

namespace Drivers
{
	Buzzer::Buzzer(uint8_t PinNo) :_PinNo(PinNo)
	{
		Vfb_SetPinMode(this->_PinNo, OUTPUT);
	}

	Buzzer::~Buzzer()
	{
	}

	void Buzzer::SetTone(unsigned int frequency,  unsigned long duration)
	{
		tone(this->_PinNo, frequency, duration);
	}

	void Buzzer::StopTone()
	{
		noTone(this->_PinNo);
	}

} /* namespace Drivers */
