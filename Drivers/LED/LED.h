#ifndef LED_H
#define LED_H

#include "HAL.h"

namespace Drivers
{
    class LED
    {
    public:
        enum class STATE
        {
            OFF = 0,
            ON = 1,
        };

        enum class BLINK_MODE
        {
            NONE = 0,
            CONTINUOUS = 1,
            COUNT_LIMITED = 2
        };

        LED(uint8_t pin, bool isPWMCapable = false);
        ~LED();

        // Basic LED control
        void On();
        void Off();
        void Toggle();
        STATE GetState();

        // Brightness control (PWM)
        void SetBrightness(uint8_t brightness); // 0-255, only works on PWM pins
        uint8_t GetBrightness() const;
        void FadeIn(unsigned long durationMs);
        void FadeOut(unsigned long durationMs);

        // Blinking functionality
        void StartBlink(unsigned long intervalMs);
        void StartBlink(unsigned long onTimeMs, unsigned long offTimeMs);
        void StartBlinkCount(unsigned long intervalMs, uint16_t count);
        void StopBlink();
        bool IsBlinking() const;

        // Pattern blinking
        void StartPattern(const unsigned long* pattern, uint8_t patternLength, bool repeat = true);

        // Call this regularly in your main loop for non-blocking operation
        void Update();

    private:
        // Hardware
        uint8_t _PinNo = 0;
        bool _isPWMPin = false;

        // State management
        STATE _currentState = STATE::OFF;
        uint8_t _brightness = 255;

        // Blinking
        BLINK_MODE _blinkMode = BLINK_MODE::NONE;
        unsigned long _blinkOnTime = 500;   // ms
        unsigned long _blinkOffTime = 500;  // ms
        unsigned long _lastBlinkTime = 0;
        uint16_t _blinkCount = 0;
        uint16_t _remainingBlinks = 0;
        bool _blinkState = false; // true = on phase, false = off phase

        // Pattern blinking
        const unsigned long* _pattern = nullptr;
        uint8_t _patternLength = 0;
        uint8_t _patternIndex = 0;
        bool _patternRepeat = false;

        // Fading
        bool _isFading = false;
        unsigned long _fadeStartTime = 0;
        unsigned long _fadeDuration = 0;
        uint8_t _fadeStartBrightness = 0;
        uint8_t _fadeTargetBrightness = 0;

        // Helper methods
        void _writePin(bool state);
        void _writePWM(uint8_t value);
        void _updateBlink();
        void _updatePattern();
        void _updateFade();
        bool _isPWMValue(uint8_t value) const;
    };

} /* namespace Drivers */

#endif /* LED_H */
