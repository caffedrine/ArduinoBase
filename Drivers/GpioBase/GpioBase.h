/*
 * GpioBase.h
 *
 *  Created on: Apr 19, 2019
 *      Author: curiosul
 */

#ifndef GPIOBASE_H_
#define GPIOBASE_H_

#include "HAL.h"

namespace Drivers
{
	class GpioBase
	{
	public:
		GpioBase(uint8_t PinNo);
		GpioBase(uint8_t PinNo, uint8_t Mode);

		void Set();
		void Clear();
		void Toggle();
		void Write(uint8_t LogicalLevel);
		uint8_t Read();
		uint16_t ReadAnalog();
		uint8_t GetPinNo();
		uint8_t GetPinMode();

		virtual ~GpioBase();
		private:
		uint8_t _PinNo = 0;
		uint8_t _Mode = INPUT;
	};

} /* namespace Drivers */

#endif /* GPIOBASE_H_ */
