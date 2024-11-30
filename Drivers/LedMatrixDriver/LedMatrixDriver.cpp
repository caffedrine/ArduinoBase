/*
 * LedMatrixDriver.cpp
 *
 *  Created on: Apr 19, 2021
 *      Author: curiosul
 */

#include "LedMatrixDriver.h"

namespace Drivers
{

	LedMatrixDriver::LedMatrixDriver(uint8_t nElementsX, uint8_t nElementsY, uint8_t ClockPin, uint8_t DataPin, uint8_t LatchPin)
	{
		this->_nElementsX = nElementsX;
		this->_nElementsY = nElementsY;

		this->_HC595 = new HC595(ClockPin, DataPin, LatchPin, 3);
	}

	LedMatrixDriver::~LedMatrixDriver()
	{
		delete(this->_HC595);
	}

	void LedMatrixDriver::SetAll()
	{
		for(int i = 0; i < this->_nElementsX; i++)
			for(int j = 0; j < this->_nElementsY; j++ )
				this->_Matrix[i][j] = 1;
	}

	void LedMatrixDriver::ClearAll()
	{
		for(int i = 0; i < this->_nElementsX; i++)
			for(int j = 0; j < this->_nElementsY; j++ )
				this->_Matrix[i][j] = 0;
	}

	void LedMatrixDriver::SetAllX(uint8_t y)
	{
		for(int i = 0; i < this->_nElementsX; i++)
		{
			this->_Matrix[i][y] = 1;
		}
	}
	void LedMatrixDriver::ClearAllX(uint8_t y)
	{
		for(int i = 0; i < this->_nElementsX; i++)
		{
			this->_Matrix[i][y] = 0;
		}
	}
	void LedMatrixDriver::SetAllY(int8_t x)
	{
		for(int i = 0; i < this->_nElementsY; i++)
		{
			this->_Matrix[x][i] = 1;
		}
	}
	void LedMatrixDriver::ClearAllY(uint8_t x)
	{
		for(int i = 0; i < this->_nElementsY; i++)
		{
			this->_Matrix[x][i] = 0;
		}
	}

	void LedMatrixDriver::SetBit(uint8_t x, uint8_t y)
	{
		this->_Matrix[x][y] = 1;
	}

	void LedMatrixDriver::ClearBit(uint8_t x, uint8_t y)
	{
		this->_Matrix[x][y] = 0;
	}

	void LedMatrixDriver::TextMatrixPositions()
	{
		if( this->_Matrix[2][0] == 0 )
		{
			this->SetAllY(2);
			this->SetAllX(3);
		}
		else
		{
			this->ClearAllX(3);
			this->ClearAllY(2);
		}

		//this->PrintMatrix();
	}

	void LedMatrixDriver::PrintMatrix()
	{
		for (int i = 0; i < this->_nElementsY; i++)
		{
			for (int j = 0; j < this->_nElementsX; j++)
			{
				Serial.print( String(this->_Matrix[j][i]) + " " );
			}
			Serial.println();
		}
		Serial.println();
	}

//	void LedMatrixDriver::LoadFrame(uint8_t *matrix)
//	{
//		for( int i = 0; i < this->_nElementsY; i++ )
//		{
//			for( int j = 0; j < this->_nElementsX; j++ )
//			{
//				this->_Matrix[i][j] = matrix[i][j];
//			}
//		}
//
//		this->PrintMatrix();
//	}

	void LedMatrixDriver::MainFunction()
	{
		// 						A			A			C
		//uint8_t regs[3] = {0b00000001, 0bxxxx1111, 0b11111101};

		// Poweroff all the leds at the beginning
		this->_RegsBuffer[0] = 0b00000000;
		this->_RegsBuffer[1] = 0b00000000;
		this->_RegsBuffer[2] = 0b11111111;

		// Calculate which LEDs shall be processed next. Multiple calls, will turn on multiple leds in parallel instead one
		// This can be used to reduce the load needed to proces the entire matrix once.
		// One function call in same loop, current will be drawn at same time only by a LED. Two calls, will have two LEDs simultaneously open and so on
		this->CalcNextPosition();
		this->CalcNextPosition();
		this->CalcNextPosition();

		// Send data to shift register and trigger main function to send data to output
		this->_HC595->WriteRaw(this->_RegsBuffer, sizeof(this->_RegsBuffer));
		this->_HC595->MainFunction();
	}

	void LedMatrixDriver::CalcNextPosition()
	{
		// By default, when entering here, LED is OFF. Only do calculus if LED shall be ON

		if( this->_Matrix[this->_CursorX][this->_CursorY] > 0 )
		{
			// Set current anod bit to ON
			if( this->_CursorX <= 3 )
			{
				this->_RegsBuffer[1] |=  (0x1 << (3 - this->_CursorX));
			}
			else
			{
				this->_RegsBuffer[0] |= (0x1 << (this->_CursorX - 4));
			}

			// Set current cathod bit to OFF
			this->_RegsBuffer[2] &= ~(0x1 << this->_CursorY);
		}

		// Increment display lines
		this->_CursorX++;
		if( this->_CursorX >= this->_nElementsX )
		{
			this->_CursorX = 0;
			this->_CursorY++;
			if( this->_CursorY >= this->_nElementsY )
			{
				this->_CursorY = 0;
			}
		}
	}

	void LedMatrixDriver::Hc595AllOff()
	{
		static uint8_t regs[3] = {0b00000000, 0b00001111, 0b11111111};
		this->_HC595->WriteRaw(regs, sizeof(regs));
	}

	void LedMatrixDriver::Hc595AllOn()
	{
		static uint8_t regs[3] = {0b11111111, 0b00000000, 0b00000000};
		this->_HC595->WriteRaw(regs, sizeof(regs));
	}

} /* namespace Drivers */
