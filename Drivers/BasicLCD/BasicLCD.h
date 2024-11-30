/*
 * BasicLCD.h
 *
 *  Created on: Apr 24, 2019
 *      Author: curiosul
 */

#ifndef BASICLCD_H_
#define BASICLCD_H_

#include <HAL.h>
#include <LiquidCrystal.h>

namespace Drivers
{
	class BasicLCD : public LiquidCrystal
	{
	public:
		BasicLCD(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
		virtual ~BasicLCD();

		void Init(uint8_t cols, uint8_t lines);
		void PrintLine(String str, uint8_t line);
		void PrintLine(uint8_t *str, uint8_t len, uint8_t line);
		void Update();

	private:
		String *_LinesBuffer = NULL;
		uint8_t _Lines = 0, _Cols = 0;
	};

} /* namespace Drivers */

#endif /* BASICLCD_H_ */
