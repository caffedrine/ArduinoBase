/*
 * LED.cpp
 *
 *  Created on: Apr 23, 2019
 *      Author: curiosul
 */

#include "LED.h"

namespace Drivers
{

	LED::LED(uint8_t Pin) : _PinNo(Pin)
	{
		Vfb_SetPinMode(this->_PinNo, OUTPUT);
		this->Off();
	}

	LED::~LED()
	{
		this->Off();
	}

	void LED::On()
	{
		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][LED] On(): Invalid pin or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}

		Vfb_DigitalWrite(this->_PinNo, HIGH);
	}

	void LED::Off()
	{
		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][LED] Off(): Invalid pin or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}

		Vfb_DigitalWrite(this->_PinNo, LOW);
	}

	void LED::Toggle()
	{
		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][LED] Toggle(): Invalid pin or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}

		Vfb_DigitalToggle(this->_PinNo);
	}

	LED::STATE LED::GetState()
	{
		if( Vfb_DigitalRead(this->_PinNo) == HIGH )
		{
			return LED::STATE::ON;
		}
		else
		{
			return LED::STATE::OFF;
		}
	}

} /* namespace Drivers */
