/*
 * HC595.h
 *
 *  Created on: Apr 19, 2019
 *      Author: curiosul
 */

#ifndef HC595_H_
#define HC595_H_

#include "HAL.h"
#include "GpioBase.h"

#ifndef HC595_DEBUG_MESSAGES
	#define HC595_DEBUG_MESSAGES	1
#endif

#ifndef HC595_DEBUG_ENABLED
	#define HC595_DEBUG_ENABLED		1
#endif

#define HC595_EXTENDED_FUNCTIONS

#define HC595_BIG_ENDIAN			1
#define HC595_SMALL_ENDIAN			0

#ifndef HC595_BIT_SHIFT_ORDER
	#define HC595_BIT_SHIFT_ORDER	LSBFIRST
#endif
#ifndef HC595_ENDIANESS
	#define HC595_ENDIANESS			HC595_SMALL_ENDIAN
#endif

namespace Drivers
{
	enum class HC595Pin
	{
		Q_A = 0,
		Q_B = 1,
		Q_C = 2,
		Q_D = 3,
		Q_E = 4,
		Q_F = 5,
		Q_G = 6,
		Q_H = 7,
	};

	class HC595
	{
	public:
		HC595(uint8_t ClockPin, uint8_t DataPin, uint8_t LatchPin, uint8_t RegsNo = 1);
		virtual ~HC595();

		void SetAll();
		void ClearAll();
		void ToggleAll();
		void WriteRaw(uint8_t *Data, uint8_t len);

		void SetBit(uint8_t bit, uint8_t RegIdx = 0);
		void ClearBit(uint8_t bit, uint8_t RegIdx = 0);
		void ToggleBit(uint8_t bit, uint8_t RegIdx = 0);
		void WriteBit(uint8_t bit, uint8_t value, uint8_t RegIdx = 0);

		void WriteByte(uint8_t Byte, uint8_t RegIdx = 0);
		void ToggleByte(uint8_t RegIdx = 0);
		void ClearByte(uint8_t RegIdx = 0);
		void SetByte(uint8_t RegIdx = 0);

#ifdef HC595_EXTENDED_FUNCTIONS
		void SetBitNo(int bit_number);
		void ClearBitNo(int bit_number);

		void SetFirstNBits(uint8_t bits_number);
		void ClearFirstNBits(uint8_t bits_number);
		void SetLastNBits(uint8_t bits_number);
		void ClearLastNBits(uint8_t bits_number);
#endif

		void MainFunction()
		{
			/* Loop through all shift registers */
			#if (HC595_ENDIANESS == HC595_BIG_ENDIAN)
				for(uint8_t reg = 0; reg < this->_RegsNo; reg++){
			#else
				static uint8_t reg;
				for(uint8_t tmpReg = (this->_RegsNo); tmpReg > 0; tmpReg--) {
				reg = tmpReg-1;
			#endif
				/* Loop through all bits from current shift register */
				for (int bit = 0; bit < 8; bit++)
				{
					#if HC595_BIT_SHIFT_ORDER == MSBFIRST
						this->_DataPin.Write(!!(this->_Buffer[reg] & (1 << (7 - bit))));
					#else
						this->_DataPin.Write(!!(this->_Buffer[reg] & (1 << bit)));
					#endif

					this->_ClockPin.Set();
					this->_ClockPin.Clear();
				}
			}

			/* Start writing session */
			this->_LatchPin.Clear();
			/* End writing session and output data */
			this->_LatchPin.Set();
		}

	private:
		GpioBase _ClockPin, _DataPin, _LatchPin;
		uint8_t _RegsNo = 1;
		/* Buffer to store current values */
		uint8_t *_Buffer;

	};

} /* namespace Drivers */

#endif /* HC595_H_ */
