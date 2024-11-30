/*
 * BasicLCD.cpp
 *
 *  Created on: Apr 24, 2019
 *      Author: curiosul
 */

#include "BasicLCD.h"

namespace Drivers
{

BasicLCD::BasicLCD(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) :
		LiquidCrystal(rs, enable, d0, d1, d2, d3)
{

}

BasicLCD::~BasicLCD()
{
}
void BasicLCD::Init(uint8_t cols, uint8_t lines)
{
	this->_Lines = lines;
	this->_Cols = cols;

	this->_LinesBuffer = new String[this->_Lines];
	for (uint8_t i = 0; i < this->_Lines; i++)
	{
		this->_LinesBuffer[i] = "";
	}

	LiquidCrystal::begin(this->_Cols, this->_Lines);
}

void BasicLCD::PrintLine(String str, uint8_t line)
{
	/* Fill with at least 8 blank lines */
	for(uint8_t i =0;  i< this->_Cols; i++)
		str += " ";

	if (str.length() > this->_Cols)
	{
		str = str.substring(0, this->_Cols);
	}

	if (line >= this->_Lines)
	{
		ERR_PRINTLN("[ERR][BasicLcd] PrintLine(): Invalid or not supported line number!");
		return;
	}

	this->_LinesBuffer[line] = str;
}

void BasicLCD::PrintLine(uint8_t *str, uint8_t len, uint8_t line)
{
	if (len > this->_Cols)
		len = this->_Cols;

	if (line >= this->_Lines)
	{
		ERR_PRINTLN("[ERR][BasicLcd] PrintLine(): Invalid or not supported line number!");
		return;
	}

	/* Convert byte to string */
	String res = "";
	for (int i = 0; i < len; i++)
	{
		res += String(str[i]);
	}

	/* Fill with at least 8 blank lines */
	for(uint8_t i = 0;  i < this->_Cols; i++)
		res += " ";

	if (res.length() > this->_Cols)
		res = res.substring(0, this->_Cols);

	this->_LinesBuffer[line] = res;
}

void BasicLCD::Update()
{
	static unsigned long PrevUpdateTimestamp = 0;

	if (micros() - PrevUpdateTimestamp >= 300000)
	{
		PrevUpdateTimestamp = micros();
		for (uint8_t line = 0; line < this->_Lines; line++)
		{
			if (this->_LinesBuffer[line] == "")
				continue;

			LiquidCrystal::setCursor(0, line);
			LiquidCrystal::print(this->_LinesBuffer[line]);
		}
	}
}

} /* namespace Drivers */
