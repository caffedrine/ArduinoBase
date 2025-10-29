#ifndef BUZZER_H
#define BUZZER_H

#include "HAL.h"

namespace Drivers
{
	class Buzzer
	{
	public:
		Buzzer(uint8_t PinNo);
		~Buzzer();
		void SetTone(unsigned int frequency,  unsigned long duration = 0);
		void StopTone();

	private:
		uint8_t _PinNo = 0;
	};

} /* namespace Drivers */

#endif /* BUZZER_H */
