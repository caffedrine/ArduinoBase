#include "IR_TxLed.h"

namespace Drivers
{
    IR_TxLed::IR_TxLed(uint8_t txPin) : 
        _txPin(txPin), _state(STATE::IDLE), _stateStartTime(0),
        _carrierFreq(38000), _transmitPower(255), _transmitBuffer(nullptr),
        _transmitLength(0), _transmitIndex(0), _lastTransmitToggle(0),
        _transmitCarrierState(false), _repeatCount(0), _currentRepeat(0),
        _txCallback(nullptr), _txCallbackEnabled(false), _debugEnabled(false)
    {
        // Initialize TX pin
        Vfb_SetPinMode(_txPin, OUTPUT);
        Vfb_DigitalWrite(_txPin, LOW);
        
        _setState(STATE::IDLE);
    }

    IR_TxLed::~IR_TxLed()
    {
        // Set TX pin to safe state
        Vfb_DigitalWrite(_txPin, LOW);
    }

    bool IR_TxLed::TransmitNEC(uint32_t code)
    {
        if (_state == STATE::TRANSMITTING)
        {
            return false;
        }

        // Allocate buffer for NEC transmission
        static uint16_t necBuffer[68];  // NEC needs max 68 timings
        uint16_t length;
        
        _generateNECCode(code, necBuffer, &length);
        _startTransmission(necBuffer, length);
        
        return true;
    }

    bool IR_TxLed::TransmitRC5(uint16_t code)
    {
        if (_state == STATE::TRANSMITTING)
        {
            return false;
        }

        static uint16_t rc5Buffer[28];  // RC5 needs max 28 timings
        uint16_t length;
        
        _generateRC5Code(code, rc5Buffer, &length);
        _startTransmission(rc5Buffer, length);
        
        return true;
    }

    bool IR_TxLed::TransmitSony(uint16_t code)
    {
        if (_state == STATE::TRANSMITTING)
        {
            return false;
        }

        static uint16_t sonyBuffer[48];  // Sony needs max 48 timings
        uint16_t length;
        
        _generateSonyCode(code, sonyBuffer, &length);
        _startTransmission(sonyBuffer, length);
        
        return true;
    }

    bool IR_TxLed::TransmitRaw(uint16_t* timings, uint16_t length)
    {
        if (_state == STATE::TRANSMITTING || timings == nullptr || length == 0)
        {
            return false;
        }

        _startTransmission(timings, length);
        return true;
    }

    bool IR_TxLed::IsTransmitting()
    {
        return (_state == STATE::TRANSMITTING);
    }

    bool IR_TxLed::IsComplete()
    {
        return (_state == STATE::COMPLETE);
    }

    void IR_TxLed::SetTransmissionCompleteCallback(TransmissionCompleteCallback callback)
    {
        _txCallback = callback;
        _txCallbackEnabled = (callback != nullptr);
    }

    void IR_TxLed::ClearTransmissionCompleteCallback()
    {
        _txCallback = nullptr;
        _txCallbackEnabled = false;
    }

    void IR_TxLed::SetCarrierFrequency(uint16_t frequency)
    {
        if (frequency >= 30000 && frequency <= 60000)
        {
            _carrierFreq = frequency;
        }
        else
        {
#if DRIVERS_DEBUG == 1
            ERR_PRINTLN("[ERR][IR_TxLed] Invalid carrier frequency (30-60kHz)");
#endif
        }
    }

    void IR_TxLed::SetTransmitPower(uint8_t power)
    {
        _transmitPower = power;
    }

    void IR_TxLed::SetRepeatCount(uint8_t repeats)
    {
        _repeatCount = repeats;
    }

    void IR_TxLed::EnableDebugging(bool enable)
    {
        _debugEnabled = enable;
    }

    IR_TxLed::STATE IR_TxLed::GetCurrentState()
    {
        return _state;
    }

    void IR_TxLed::Update()
    {
        switch (_state)
        {
            case STATE::TRANSMITTING:
                _updateTransmission();
                break;

            case STATE::COMPLETE:
                // Auto-return to idle after completion
                _setState(STATE::IDLE);
                break;

            case STATE::ERROR:
                // Auto-return to idle after error
                _setState(STATE::IDLE);
                break;

            case STATE::IDLE:
                // Nothing to do in idle state
                break;
        }
    }

    // Private Methods

    void IR_TxLed::_startTransmission(uint16_t* timings, uint16_t length)
    {
        _transmitBuffer = timings;
        _transmitLength = length;
        _transmitIndex = 0;
        _currentRepeat = 0;
        _lastTransmitToggle = micros();
        _transmitCarrierState = false;
        
        _setState(STATE::TRANSMITTING);
    }

    void IR_TxLed::_updateTransmission()
    {
        unsigned long currentTime = micros();
        
        // Check if we've completed all timings
        if (_transmitIndex >= _transmitLength)
        {
            // Check if we need to repeat
            if (_currentRepeat < _repeatCount)
            {
                _currentRepeat++;
                _transmitIndex = 0;
                _lastTransmitToggle = currentTime;
                
                // Add gap between repeats (typically 45ms for NEC)
                static unsigned long repeatGapStart = 0;
                if (repeatGapStart == 0)
                {
                    repeatGapStart = currentTime;
                    Vfb_DigitalWrite(_txPin, LOW);
                    return;
                }
                
                if (currentTime - repeatGapStart >= 45000)  // 45ms gap
                {
                    repeatGapStart = 0;
                    _lastTransmitToggle = currentTime;
                }
                else
                {
                    return;  // Still in gap period
                }
            }
            else
            {
                _completeTransmission(true);
                return;
            }
        }

        // Check if it's time to move to next timing
        unsigned long duration = _transmitBuffer[_transmitIndex];
        if (currentTime - _lastTransmitToggle >= duration)
        {
            _transmitIndex++;
            _lastTransmitToggle = currentTime;
        }
        else
        {
            // Generate carrier signal if we're in a mark period (odd indices)
            if (_transmitIndex % 2 == 0)  // Even index = mark (carrier)
            {
                _generateCarrierPulse(100);  // Generate carrier for 100us
            }
            else  // Odd index = space (no carrier)
            {
                Vfb_DigitalWrite(_txPin, LOW);
            }
        }
    }

    void IR_TxLed::_generateCarrierPulse(unsigned long durationUs)
    {
        static unsigned long lastCarrierToggle = 0;
        static bool carrierHigh = false;
        
        unsigned long currentTime = micros();
        unsigned long halfPeriod = 1000000UL / (_carrierFreq * 2);
        
        if (currentTime - lastCarrierToggle >= halfPeriod)
        {
            carrierHigh = !carrierHigh;
            
            if (carrierHigh)
            {
                // Use PWM for power control if needed
                if (_transmitPower == 255)
                {
                    Vfb_DigitalWrite(_txPin, HIGH);
                }
                else
                {
                    // Could implement PWM here for power control
                    Vfb_DigitalWrite(_txPin, HIGH);
                }
            }
            else
            {
                Vfb_DigitalWrite(_txPin, LOW);
            }
            
            lastCarrierToggle = currentTime;
        }
    }

    void IR_TxLed::_completeTransmission(bool success)
    {
        Vfb_DigitalWrite(_txPin, LOW);
        _setState(success ? STATE::COMPLETE : STATE::ERROR);
        
        if (_txCallbackEnabled && _txCallback != nullptr)
        {
            _txCallback(success);
        }
    }

    void IR_TxLed::_setState(STATE newState)
    {
        _state = newState;
        _stateStartTime = millis();
    }

    void IR_TxLed::_debugPrint(const char* message)
    {
        if (_debugEnabled)
        {
#if DRIVERS_DEBUG == 1
            ERR_PRINT("[DEBUG][IR_TxLed] ");
            ERR_PRINT(message);
#endif
        }
    }

    void IR_TxLed::_debugPrintln(const char* message)
    {
        if (_debugEnabled)
        {
#if DRIVERS_DEBUG == 1
            ERR_PRINT("[DEBUG][IR_TxLed] ");
            ERR_PRINTLN(message);
#endif
        }
    }

    // Protocol Generation Methods

    void IR_TxLed::_generateNECCode(uint32_t code, uint16_t* buffer, uint16_t* length)
    {
        uint16_t index = 0;
        
        // Start pulse: 9ms mark + 4.5ms space
        buffer[index++] = 9000;
        buffer[index++] = 4500;
        
        // 32 data bits
        for (int i = 31; i >= 0; i--)
        {
            buffer[index++] = 562;  // Mark (always 562µs)
            
            if (code & (1UL << i))
            {
                buffer[index++] = 1687;  // Space for '1' (1687µs)
            }
            else
            {
                buffer[index++] = 562;   // Space for '0' (562µs)
            }
        }
        
        // Final mark
        buffer[index++] = 562;
        
        *length = index;
    }

    void IR_TxLed::_generateRC5Code(uint16_t code, uint16_t* buffer, uint16_t* length)
    {
        uint16_t index = 0;
        uint16_t bitTime = 889;  // 889µs per half bit (1.778ms total bit time)
        
        // RC5 has 14 bits: 2 start bits + 1 toggle bit + 5 address + 6 command
        // Start bits are always '11'
        uint16_t fullCode = 0x3000 | (code & 0x0FFF);  // Add start bits
        
        for (int i = 13; i >= 0; i--)
        {
            if (fullCode & (1 << i))
            {
                // '1' = space then mark in Manchester encoding
                buffer[index++] = bitTime;  // Space
                buffer[index++] = bitTime;  // Mark
            }
            else
            {
                // '0' = mark then space in Manchester encoding
                buffer[index++] = bitTime;  // Mark
                buffer[index++] = bitTime;  // Space
            }
        }
        
        *length = index;
    }

    void IR_TxLed::_generateSonyCode(uint16_t code, uint16_t* buffer, uint16_t* length)
    {
        uint16_t index = 0;
        
        // Start pulse: 2.4ms mark + 600µs space
        buffer[index++] = 2400;
        buffer[index++] = 600;
        
        // 12 data bits (SIRC-12)
        for (int i = 0; i < 12; i++)  // Sony sends LSB first
        {
            if (code & (1 << i))
            {
                buffer[index++] = 1200;  // Mark for '1' (1.2ms)
                buffer[index++] = 600;   // Space (600µs)
            }
            else
            {
                buffer[index++] = 600;   // Mark for '0' (600µs)
                buffer[index++] = 600;   // Space (600µs)
            }
        }
        
        *length = index;
    }

} /* namespace Drivers */
