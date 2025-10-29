#include "Gpio.h"

namespace Drivers
{
	Gpio::Gpio(uint8_t PinNo) :_PinNo(PinNo), _Mode(INPUT)
	{
		Vfb_SetPinMode(this->_PinNo, this->_Mode);
	}

	Gpio::Gpio(uint8_t PinNo, uint8_t Mode) : _PinNo(PinNo), _Mode(Mode)
	{
		Vfb_SetPinMode(this->_PinNo, this->_Mode);
	}

	Gpio::~Gpio()
	{
	}

	void Gpio::Set()
	{
			if(this->_Mode == INPUT)
			{
				#if DRIVERS_DEBUG == 1
					ERR_PRINT("[ERR][GpioBase] Set(): Can't SET a pin set as INPUT: ");
					ERR_PRINTLN(this->_PinNo);
				#endif
				return;
			}

			if(this->_PinNo <= 0 )
			{
				#if DRIVERS_DEBUG == 1
					ERR_PRINT("[ERR][GpioBase] Set(): Setting invalid pin or not initialized: ");
					ERR_PRINTLN(this->_PinNo);
				#endif
				return;
			}
			Vfb_DigitalWrite(this->_PinNo, HIGH);
	}

	void Gpio::Clear()
	{
		if(this->_Mode == INPUT)
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] Clear(): Can't CLEAR a pin set as INPUT: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}

		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] Clear(): Clearing invalid pin or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}
		Vfb_DigitalWrite(this->_PinNo, LOW);
	}

	void Gpio::Write(uint8_t LogicalLevel)
	{
		if(this->_Mode == INPUT)
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] Write(): Can't WRITE a pin set as INPUT: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}

		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] Write(): Writing invalid pin or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}

		Vfb_DigitalWrite(this->_PinNo, LogicalLevel);
	}

	void Gpio::Toggle()
	{
		if(this->_Mode == INPUT)
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] Toggle(): Can't TOGGLE a pin set as INPUT: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}

		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] Toggle(): Toggling invalid pin or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
			return;
		}
		Vfb_DigitalToggle(this->_PinNo);
	}

	uint8_t Gpio::Read()
	{
		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] Read(): Invalid pin number or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
		}
		return Vfb_DigitalRead(this->_PinNo);
	}

	uint16_t Gpio::ReadAnalog()
	{
		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] ReadAnalog(): Invalid pin number or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
		}
		return (uint16_t)Vfb_AnalogRead(this->_PinNo);
	}

	uint8_t Gpio::GetPinNo()
	{
		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] GetPinNo(): Invalid pin number or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
		}
		return this->_PinNo;
	}

	uint8_t Gpio::GetPinMode()
	{
		if(this->_PinNo <= 0 )
		{
			#if DRIVERS_DEBUG == 1
				ERR_PRINT("[ERR][GpioBase] GetPinNo(): Invalid pin number or not initialized: ");
				ERR_PRINTLN(this->_PinNo);
			#endif
		}
		return this->_Mode;
	}

} /* namespace Drivers */
