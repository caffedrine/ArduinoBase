/*
 * HC595.cpp
 *
 *  Created on: Apr 19, 2019
 *      Author: curiosul
 */

#include "HC595.h"

namespace Drivers
{
	HC595::HC595(uint8_t ClockPin, uint8_t DataPin, uint8_t LatchPin, uint8_t RegNo) : _ClockPin(ClockPin, OUTPUT), _DataPin(DataPin, OUTPUT), _LatchPin(LatchPin, OUTPUT), _RegsNo(RegNo)
	{
		this->_Buffer = (uint8_t *)malloc(this->_RegsNo*sizeof(uint8_t));
	}

	HC595::~HC595()
	{
		free(this->_Buffer);
	}

	void HC595::SetAll()
	{
		for(uint8_t i = 0; i < this->_RegsNo; i++)
		{
			this->_Buffer[i] = 0xFF;
		}
	}

	void HC595::ClearAll()
	{
		for(uint8_t i = 0; i < this->_RegsNo; i++)
		{
			this->_Buffer[i] = 0x00;
		}
	}

	void HC595::ToggleAll()
	{
		for(uint8_t i = 0; i < this->_RegsNo; i++)
		{
			this->_Buffer[i] = ~this->_Buffer[i];
		}
	}

	void HC595::WriteRaw(uint8_t *Data, uint8_t len)
	{
		if(len > this->_RegsNo)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] WriteRaw(): Invalid data length: ");
				ERR_PRINTLN(len);
			#endif
			return;
		}

		for(uint8_t i = 0; i < this->_RegsNo; i++)
		{
			this->_Buffer[i] = Data[i];
		}
	}

	void HC595::SetBit(uint8_t bit, uint8_t RegIndex)
	{
		if(RegIndex >= this->_RegsNo || (uint8_t)bit >= 8)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] SetBit(): Invalid register index: ");
				ERR_PRINTLN(RegIndex);
			#endif
			return;
		}


		//DBG_PRINTLN("reg[" + String(RegIndex) + "][" + String(bit) + "] = 1");
		this->_Buffer[RegIndex] |= (1 << (uint8_t)bit);
	}

	void HC595::ClearBit(uint8_t bit, uint8_t RegIndex)
	{
		if(RegIndex >= this->_RegsNo || (uint8_t)bit >= 8)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] ClearBit(): Invalid register index: ");
				ERR_PRINTLN(RegIndex);
			#endif
			return;
		}

		//DBG_PRINTLN("reg[" + String(RegIndex) + "][" + String(bit) + "] = 0");
		this->_Buffer[RegIndex] &= (~(1 << (uint8_t)bit));
	}

	void HC595::ToggleBit(uint8_t bit, uint8_t RegIndex)
	{
		if(RegIndex >= this->_RegsNo || (uint8_t)bit >= 8)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] ToggleBit(): Invalid register index: ");
				ERR_PRINTLN(RegIndex);
			#endif
			return;
		}
		this->_Buffer[RegIndex] ^= (1 << (uint8_t)bit);
	}

	void HC595::WriteBit(uint8_t bit, uint8_t value, uint8_t RegIdx)
	{
		if(RegIdx >= this->_RegsNo || (uint8_t)bit >= 8)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] WriteBit(): Invalid register index: ");
				ERR_PRINTLN(RegIdx);
			#endif
			return;
		}
		if(value == 1)
		{
			this->SetBit(bit, RegIdx);
		}
		else
		{
			this->ClearBit(bit, RegIdx);
		}
	}

	void HC595::WriteByte(uint8_t Byte, uint8_t RegIndex)
	{
		if(RegIndex >= this->_RegsNo)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] WriteByte(): Invalid register index: ");
				ERR_PRINTLN(RegIndex);
			#endif
			return;
		}
		this->_Buffer[RegIndex] = Byte;
	}

	void HC595::ToggleByte(uint8_t RegIdx)
	{
		if(RegIdx >= this->_RegsNo)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] ToggleByte(): Invalid register index: ");
				ERR_PRINTLN(RegIdx);
			#endif
			return;
		}

		this->_Buffer[RegIdx] = ~this->_Buffer[RegIdx];
	}

	void HC595::ClearByte(uint8_t RegIdx)
	{
		if(RegIdx >= this->_RegsNo)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] ClearByte(): Invalid register index: ");
				ERR_PRINTLN(RegIdx);
			#endif
			return;
		}

		this->_Buffer[RegIdx] = 0x00;
	}

	void HC595::SetByte(uint8_t RegIdx)
	{
		if(RegIdx >= this->_RegsNo)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] SetByte(): Invalid register index: ");
				ERR_PRINTLN(RegIdx);
			#endif
			return;
		}

		this->_Buffer[RegIdx] = 0xFF;
	}

#ifdef HC595_EXTENDED_FUNCTIONS
	void HC595::SetBitNo(int bit_number)
	{
		// Calculate on which register index this one belongs
		uint8_t RegIndex = 0;
		if( bit_number > 7 )
		{
			RegIndex = (bit_number/8);
		}

		if (bit_number < 0)
		{
			RegIndex = this->_RegsNo - RegIndex - 1 ;
		}

		// Validate register index
		if(RegIndex >= this->_RegsNo)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] SetBitNo(): Invalid register index calculated ");
				ERR_PRINT(RegIndex);
				ERR_PRINT(" for bit number ");
				ERR_PRINTLN(bit_number);
			#endif
			return;
		}

		// Validate bit range
		if(abs(bit_number) > (this->_RegsNo * 8) - 1)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] SetBitNo(): Invalid bit number ");
				ERR_PRINT(bit_number);
				ERR_PRINT(", max  ");
				ERR_PRINTLN(this->_RegsNo * 8 - 1);
			#endif
			return;
		}

		if( bit_number >= 0 )
		{
			if( bit_number == 0 )
			{
				this->SetBit(0, RegIndex);
			}
			else
			{
				this->SetBit((uint8_t)(bit_number%8), RegIndex);
			}
		}
		else
		{
			this->SetBit((uint8_t)(8 - abs(bit_number))%8, RegIndex);
		}
	}
	void HC595::ClearBitNo(int bit_number)
	{
		// Calculate on which register index this one belongs
		uint8_t RegIndex = 0;
		if( bit_number > 7 )
		{
			RegIndex = (bit_number/8);
		}

		if (bit_number < 0)
		{
			RegIndex = this->_RegsNo - RegIndex - 1;
		}

		// Validate register index
		if(RegIndex >= this->_RegsNo)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] ClearBitNo(): Invalid register index calculated ");
				ERR_PRINT(RegIndex);
				ERR_PRINT(" for bit number ");
				ERR_PRINTLN(bit_number);
			#endif
			return;
		}

		// Validate bit range
		if(abs(bit_number) > (this->_RegsNo * 8) - 1)
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] ClearBitNo(): Invalid bit number ");
				ERR_PRINT(bit_number);
				ERR_PRINT(", max  ");
				ERR_PRINTLN(this->_RegsNo * 8 - 1);
			#endif
			return;
		}

		if( bit_number >= 0 )
		{
			if( bit_number == 0 )
			{
				this->ClearBit(0, RegIndex);
			}
			else
			{
				this->ClearBit(uint8_t(bit_number%8), RegIndex);
			}
		}
		else
		{
			this->ClearBit(uint8_t((8-abs(bit_number))%8), RegIndex);
		}
	}

	void HC595::SetFirstNBits(uint8_t bits_number)
	{
		// Validate bits range
		if(bits_number > (this->_RegsNo * 8))
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] SetFirstNBits(): Invalid bits number ");
				ERR_PRINT(bits_number);
				ERR_PRINT(", max  ");
				ERR_PRINTLN(this->_RegsNo * 8);
			#endif
			return;
		}


		for( uint8_t i = 0; i < bits_number; i++ )
		{
			this->SetBitNo(i);
		}
	}
	void HC595::ClearFirstNBits(uint8_t bits_number)
	{
		// Validate bit range
		if(bits_number > (this->_RegsNo * 8))
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] ClearFirstNBits(): Invalid bits number ");
				ERR_PRINT(bits_number);
				ERR_PRINT(", max  ");
				ERR_PRINTLN(this->_RegsNo * 8 - 1);
			#endif
			return;
		}

		for( uint8_t i = 0; i < bits_number; i++ )
		{
			this->ClearBitNo(i);
		}
	}
	void HC595::SetLastNBits(uint8_t bits_number)
	{
		// Validate bit range
		if(bits_number > (this->_RegsNo * 8))
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] SetLastNBits(): Invalid bits number ");
				ERR_PRINT(bits_number);
				ERR_PRINT(", max  ");
				ERR_PRINTLN(this->_RegsNo * 8);
			#endif
			return;
		}

		for( int i = (this->_RegsNo * 8) - 1; i >= ((this->_RegsNo * 8) - bits_number); i-- )
		{
			this->SetBitNo(i);
		}
	}
	void HC595::ClearLastNBits(uint8_t bits_number)
	{
		// Validate bit range
		if(bits_number > (this->_RegsNo * 8))
		{
			#if HC595_DEBUG_ENABLED ==1
				ERR_PRINT("[ERR][HC595] ClearLastNBits(): Invalid bits number ");
				ERR_PRINT(bits_number);
				ERR_PRINT(", max  ");
				ERR_PRINTLN(this->_RegsNo * 8);
			#endif
			return;
		}

		for( int i = (this->_RegsNo * 8) - 1; i >= ((this->_RegsNo * 8) - bits_number); i-- )
		{
			this->ClearBitNo(i);
		}
	}
#endif

} /* namespace Drivers */
