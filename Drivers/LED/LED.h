/*
 * LED.h
 *
 *  Created on: Apr 23, 2019
 *      Author: curiosul
 */

#ifndef LED_H_
#define LED_H_

#include "GpioBase.h"
#include "HAL.h"

namespace Drivers
{
	class LED
	{
	public:
		enum class STATE
		{
			OFF = 0,
			ON = 1,
		};

		LED(uint8_t pin);
		virtual ~LED();

		void On();
		void Off();
		void Toggle();
		STATE GetState();
	private:
		uint8_t _PinNo = 0;
	};

} /* namespace Drivers */

#endif /* LED_H_ */
