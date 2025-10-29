#include "LED.h"

namespace Drivers
{

LED::LED(uint8_t pin, bool isPWMCapable): _PinNo(pin), _isPWMPin(isPWMCapable)
{
	Vfb_SetPinMode(this->_PinNo, OUTPUT);
	this->Off();
}

LED::~LED()
{
	this->StopBlink();
	this->Off();
}

void LED::On()
{
	if (this->_PinNo <= 0)
	{
		#if DRIVERS_DEBUG == 1
            ERR_PRINT("[ERR][LED] On(): Invalid pin or not initialized: ");
            ERR_PRINTLN(this->_PinNo);
        #endif
		return;
	}

	this->StopBlink();
	this->_currentState = STATE::ON;

	if (_isPWMPin)
	{
		this->_writePWM(_brightness);
	}
	else
	{
		this->_writePin(true);
	}
}

void LED::Off()
{
	if (this->_PinNo <= 0)
	{
		#if DRIVERS_DEBUG == 1
            ERR_PRINT("[ERR][LED] Off(): Invalid pin or not initialized: ");
            ERR_PRINTLN(this->_PinNo);
        #endif
		return;
	}

	this->StopBlink();
	this->_currentState = STATE::OFF;

	if (_isPWMPin)
	{
		this->_writePWM(0);
	}
	else
	{
		this->_writePin(false);
	}
}

void LED::Toggle()
{
	if (this->_PinNo <= 0)
	{
		#if DRIVERS_DEBUG == 1
            ERR_PRINT("[ERR][LED] Toggle(): Invalid pin or not initialized: ");
            ERR_PRINTLN(this->_PinNo);
        #endif
		return;
	}

	if (_currentState == STATE::ON)
	{
		this->Off();
	}
	else
	{
		this->On();
	}
}

LED::STATE LED::GetState()
{
	return _currentState;
}

void LED::SetBrightness(uint8_t brightness)
{
	_brightness = brightness;

	// If LED is currently on and we have PWM capability, update immediately
	if (_currentState == STATE::ON && _isPWMPin)
	{
		this->_writePWM(_brightness);
	}

	#if DRIVERS_DEBUG == 1
        if (!_isPWMPin && brightness != 255 && brightness != 0) {
            ERR_PRINTLN("[WARN][LED] SetBrightness(): Pin does not support PWM, brightness will be ignored");
        }
    #endif
}

uint8_t LED::GetBrightness() const
{
	return _brightness;
}

void LED::FadeIn(unsigned long durationMs)
{
	if (!_isPWMPin)
	{
#if DRIVERS_DEBUG == 1
            ERR_PRINTLN("[ERR][LED] FadeIn(): PWM not supported on this pin");
        #endif
		return;
	}

	_isFading = true;
	_fadeStartTime = millis();
	_fadeDuration = durationMs;
	_fadeStartBrightness = 0;
	_fadeTargetBrightness = _brightness;
	_currentState = STATE::ON;
}

void LED::FadeOut(unsigned long durationMs)
{
	if (!_isPWMPin)
	{
		#if DRIVERS_DEBUG == 1
            ERR_PRINTLN("[ERR][LED] FadeOut(): PWM not supported on this pin");
        #endif
		return;
	}

	_isFading = true;
	_fadeStartTime = millis();
	_fadeDuration = durationMs;
	_fadeStartBrightness = _brightness;
	_fadeTargetBrightness = 0;
}

void LED::StartBlink(unsigned long intervalMs)
{
	StartBlink(intervalMs, intervalMs);
}

void LED::StartBlink(unsigned long onTimeMs, unsigned long offTimeMs)
{
	_blinkMode = BLINK_MODE::CONTINUOUS;
	_blinkOnTime = onTimeMs;
	_blinkOffTime = offTimeMs;
	_lastBlinkTime = millis();
	_blinkState = ((GetState() == STATE::ON) ? true : false); // Start with current state
	_isFading = false; // Stop any ongoing fade
}

void LED::StartBlinkCount(unsigned long intervalMs, uint16_t count)
{
	_blinkMode = BLINK_MODE::COUNT_LIMITED;
	_blinkOnTime = intervalMs;
	_blinkOffTime = intervalMs;
	_blinkCount = count;
	_remainingBlinks = count * 2; // Each blink is on+off
	_lastBlinkTime = millis();
	_blinkState = false;
	_isFading = false;
}

void LED::StartPattern(const unsigned long *pattern, uint8_t patternLength,
		bool repeat)
{
	if (pattern == nullptr || patternLength == 0)
	{
		#if DRIVERS_DEBUG == 1
            ERR_PRINTLN("[ERR][LED] StartPattern(): Invalid pattern");
        #endif
		return;
	}

	_blinkMode = BLINK_MODE::CONTINUOUS; // We'll use this for patterns too
	_pattern = pattern;
	_patternLength = patternLength;
	_patternIndex = 0;
	_patternRepeat = repeat;
	_lastBlinkTime = millis();
	_blinkState = false;
	_isFading = false;
}

void LED::StopBlink()
{
	_blinkMode = BLINK_MODE::NONE;
	_pattern = nullptr;
	_isFading = false;
}

bool LED::IsBlinking() const
{
	return _blinkMode != BLINK_MODE::NONE || _isFading;
}

void LED::Update()
{
	if (_isFading)
	{
		_updateFade();
	}
	else if (_pattern != nullptr)
	{
		_updatePattern();
	}
	else if (_blinkMode != BLINK_MODE::NONE)
	{
		_updateBlink();
	}
}

// Private helper methods

void LED::_writePin(bool state)
{
	if (state)
	{
		Vfb_DigitalWrite(_PinNo, HIGH);
		_currentState = STATE::ON;
	}
	else
	{
		Vfb_DigitalWrite(_PinNo, LOW);
		_currentState = STATE::OFF;
	}
}

void LED::_writePWM(uint8_t value)
{
	// Assuming your HAL has an analog write function
	// You might need to adapt this to your specific HAL implementation
	Vfb_AnalogWrite(_PinNo, value);

	if (value > 0)
	{
		_currentState = STATE::ON;
	}
	else
	{
		_currentState = STATE::OFF;
	}
}

void LED::_updateBlink()
{
	unsigned long currentTime = millis();
	unsigned long targetTime;

	if (_blinkState)
	{
		targetTime = _lastBlinkTime + _blinkOnTime;
	}
	else
	{
		targetTime = _lastBlinkTime + _blinkOffTime;
	}

	if (currentTime >= targetTime)
	{
		_blinkState = !_blinkState;
		_lastBlinkTime = currentTime;

		if (_blinkState)
		{
			// Turn on
			if (_isPWMPin)
			{
				_writePWM (_brightness);
			}
			else
			{
				_writePin(true);
			}
		}
		else
		{
			// Turn off
			if (_isPWMPin)
			{
				_writePWM(0);
			}
			else
			{
				_writePin(false);
			}
		}

		// Handle count-limited blinking
		if (_blinkMode == BLINK_MODE::COUNT_LIMITED)
		{
			_remainingBlinks--;
			if (_remainingBlinks <= 0)
			{
				StopBlink();
				Off(); // Ensure LED ends in off state
			}
		}
	}
}

void LED::_updatePattern()
{
	unsigned long currentTime = millis();
	unsigned long targetTime = _lastBlinkTime + _pattern[_patternIndex];

	if (currentTime >= targetTime)
	{
		_lastBlinkTime = currentTime;
		_patternIndex++;

		// Toggle state on each pattern step
		_blinkState = !_blinkState;

		if (_blinkState)
		{
			if (_isPWMPin)
			{
				_writePWM (_brightness);
			}
			else
			{
				_writePin(true);
			}
		}
		else
		{
			if (_isPWMPin)
			{
				_writePWM(0);
			}
			else
			{
				_writePin(false);
			}
		}

		// Check if pattern is complete
		if (_patternIndex >= _patternLength)
		{
			if (_patternRepeat)
			{
				_patternIndex = 0; // Restart pattern
			}
			else
			{
				StopBlink();
				Off(); // End in off state
			}
		}
	}
}

void LED::_updateFade()
{
	unsigned long currentTime = millis();
	unsigned long elapsedTime = currentTime - _fadeStartTime;

	if (elapsedTime >= _fadeDuration)
	{
		// Fade complete
		_isFading = false;
		_writePWM (_fadeTargetBrightness);
		if (_fadeTargetBrightness == 0)
		{
			_currentState = STATE::OFF;
		}
	}
	else
	{
		// Calculate current brightness based on fade progress
		float progress = (float) elapsedTime / (float) _fadeDuration;
		uint8_t currentBrightness = _fadeStartBrightness
				+ (progress * (_fadeTargetBrightness - _fadeStartBrightness));
		_writePWM(currentBrightness);
	}
}

bool LED::_isPWMValue(uint8_t value) const
{
	return _isPWMPin && (value > 0 && value < 255);
}

} /* namespace Drivers */
