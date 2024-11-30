/*
 * LedMatrixDriver.h
 *
 *  Created on: Apr 19, 2019
 *      Author: curiosul
 */

#ifndef LedMatrixDriver_H_
#define LedMatrixDriver_H_

#include "HAL.h"
#include "GpioBase.h"
#include "HC595.h"

namespace Drivers
{
	class LedMatrixDriver
	{
	public:
		// Maximum matrix sizes
		static const uint8_t MATRIX_MAX_X_ELEMENTS = 12;
		static const uint8_t MATRIX_MAX_Y_ELEMENTS = 12;

		LedMatrixDriver(uint8_t nCathods, uint8_t nAnods, uint8_t ClockPin, uint8_t DataPin, uint8_t LatchPin);
		virtual ~LedMatrixDriver();

		void SetAll();
		void ClearAll();

		void SetAllX(uint8_t y);
		void ClearAllX(uint8_t y);
		void SetAllY(int8_t x);
		void ClearAllY(uint8_t x);
		void SetBit(uint8_t x, uint8_t y);
		void ClearBit(uint8_t x, uint8_t y);

		void LoadFrame(uint8_t matrix[MATRIX_MAX_X_ELEMENTS][MATRIX_MAX_Y_ELEMENTS])
		{
			for( int i = 0; i < this->_nElementsX; i++ )
			{
				for( int j = 0; j < this->_nElementsY; j++ )
				{
					this->_Matrix[i][j] = matrix[j][i];
				}
			}

			//this->PrintMatrix();
		}

		void TextMatrixPositions();
		void PrintMatrix();

		void MainFunction();

	private:
		uint8_t _nElementsY, _nElementsX;
		/* Buffer to store current values */
		int _Matrix[MATRIX_MAX_X_ELEMENTS][MATRIX_MAX_Y_ELEMENTS];
		/* Shift registers used to output the data */
		HC595 *_HC595;
		uint8_t _RegsBuffer[3] = {0};
		/* Matrix writing cursor */
		uint8_t _CursorX = 0, _CursorY = 0;

		void CalcNextPosition();

		void SetAllAnodsStates(uint8_t state);
		void SetAllCathodsState(uint8_t state);
		void SetSingleAnodState(uint8_t anod_no, uint8_t state);
		void SetSingleCathodState(uint8_t cathod_no, uint8_t state);

		void Hc595AllOff();
		void Hc595AllOn();
	};

} /* namespace Drivers */

#endif /* LedMatrixDriver_H_ */
