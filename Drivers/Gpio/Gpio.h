#ifndef GPIO_H
#define GPIO_H

#include "HAL.h"

namespace Drivers
{
	class Gpio
	{
		public:
			Gpio(uint8_t PinNo);
			Gpio(uint8_t PinNo, uint8_t Mode);
			virtual ~Gpio();

			void Set();
			void Clear();
			void Toggle();
			void Write(uint8_t LogicalLevel);
			uint8_t Read();
			uint16_t ReadAnalog();
			uint8_t GetPinNo();
			uint8_t GetPinMode();

		private:
			uint8_t _PinNo = 0u;
			uint8_t _Mode = INPUT;
	};

} /* namespace Drivers */

#endif /* GPIO_H */
