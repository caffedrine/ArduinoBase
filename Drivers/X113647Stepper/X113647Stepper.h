/**
 * Author: curiosul
 */

#ifndef _X113647Stepper_h_
#define _X113647Stepper_h_

#include "Arduino.h"

class X11Stepper
{
public:
	enum class DIRECTION
	{
		FORWARD = 0,
		BACKWARD = 1,
	};

	X11Stepper(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4) : _pinIn1(in1), _pinIn2(in2), _pinIn3(in3), _pinIn4(in4)
	{
		pinMode(this->_pinIn1, OUTPUT);
		pinMode(this->_pinIn2, OUTPUT);
		pinMode(this->_pinIn3, OUTPUT);
		pinMode(this->_pinIn4, OUTPUT);
	}

	void StepNext()
	{
		if(this->_CurrentDirection == DIRECTION::FORWARD)
		{
			if( (this->_CurrentStep++)>=(sizeof(this->STEPS)-1))
				this->_CurrentStep = 0;
		}
		else if(this->_CurrentDirection == DIRECTION::BACKWARD)
		{
			if(this->_CurrentStep == 0)
				this->_CurrentStep = sizeof(this->STEPS)-1;
			else
				this->_CurrentStep--;
		}
		this->_Step(this->_CurrentStep);
	}

	void Stop()
	{
		digitalWrite( this->_pinIn1, 0 );
		digitalWrite( this->_pinIn2, 0 );
		digitalWrite( this->_pinIn3, 0 );
		digitalWrite( this->_pinIn4, 0 );
	}

	void SetDirection(DIRECTION dir)
	{
		this->_CurrentDirection = dir;
	}

private:
	const uint8_t STEPS[4] = {0b00001100, 0b00000110, 0b00000011, 0b00001001};

	uint8_t _pinIn1, _pinIn2, _pinIn3, _pinIn4;
	uint8_t _CurrentStep = 0;
	DIRECTION _CurrentDirection = DIRECTION::FORWARD;

	void _Step(uint8_t STEP_NO)
	{
		digitalWrite( this->_pinIn1, 0b00001000 & STEPS[STEP_NO] );
		digitalWrite( this->_pinIn2, 0b00000100 & STEPS[STEP_NO] );
		digitalWrite( this->_pinIn3, 0b00000010 & STEPS[STEP_NO] );
		digitalWrite( this->_pinIn4, 0b00000001 & STEPS[STEP_NO] );
	}
};

#endif
