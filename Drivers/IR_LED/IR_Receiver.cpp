#include "IR_Receiver.h"

namespace Drivers
{
    // Static instance pointer for interrupt handling
    IR_Receiver* IR_Receiver::_instance = nullptr;

    IR_Receiver::IR_Receiver(uint8_t rxPin) :
        _rxPin(rxPin), _state(STATE::IDLE), _stateStartTime(0), _timeoutMs(1000),
        _lastEdgeTime(0), _pulseIndex(0), _receiving(false), _dataReady(false),
        _receivedCode(0), _receivedProtocol(PROTOCOL::UNKNOWN), _decodingLength(0),
        _minPulseWidth(50), _maxPulseWidth(10000), _rxCallback(nullptr),
        _rxCallbackEnabled(false), _periodicRxMode(false), _debugEnabled(false)
    {
        // Set static instance for interrupt handling
        _instance = this;

        // Initialize RX pin
        Vfb_SetPinMode(_rxPin, INPUT);
        
        _initializeReceiver();
        _setState(STATE::IDLE);
    }

    IR_Receiver::~IR_Receiver()
    {
        StopReceiving();
        StopPeriodicReceiving();
        
        // Clear static instance
        if (_instance == this)
        {
            _instance = nullptr;
        }
    }

    bool IR_Receiver::StartReceiving()
    {
        if (_state == STATE::RECEIVING)
        {
            return false;
        }

        _setState(STATE::RECEIVING);
        _pulseIndex = 0;
        _receiving = true;
        _dataReady = false;
        
        // Attach interrupt for RX pin (both edges to capture mark and space)
        attachInterrupt(digitalPinToInterrupt(_rxPin), _rxInterrupt, CHANGE);
        
        return true;
    }

    bool IR_Receiver::StopReceiving()
    {
        _receiving = false;
        _dataReady = false;
        detachInterrupt(digitalPinToInterrupt(_rxPin));
        
        if (_state == STATE::RECEIVING)
        {
            _setState(STATE::IDLE);
        }
        
        return true;
    }

    bool IR_Receiver::IsReceiving()
    {
        return (_state == STATE::RECEIVING);
    }

    bool IR_Receiver::IsSignalDetected()
    {
        return (_state == STATE::SIGNAL_DETECTED);
    }

    uint32_t IR_Receiver::GetReceivedCode()
    {
        return _receivedCode;
    }

    IR_Receiver::PROTOCOL IR_Receiver::GetReceivedProtocol()
    {
        return _receivedProtocol;
    }

    uint16_t* IR_Receiver::GetRawData(uint16_t* length)
    {
        if (length != nullptr)
        {
            *length = _decodingLength;
        }
        return _decodingBuffer;
    }

    void IR_Receiver::SetSignalReceivedCallback(SignalReceivedCallback callback)
    {
        _rxCallback = callback;
        _rxCallbackEnabled = (callback != nullptr);
    }

    void IR_Receiver::ClearSignalReceivedCallback()
    {
        _rxCallback = nullptr;
        _rxCallbackEnabled = false;
    }

    bool IR_Receiver::StartPeriodicReceiving(SignalReceivedCallback callback)
    {
        SetSignalReceivedCallback(callback);
        _periodicRxMode = true;
        
        return StartReceiving();
    }

    void IR_Receiver::StopPeriodicReceiving()
    {
        _periodicRxMode = false;
        StopReceiving();
    }

    bool IR_Receiver::IsPeriodicModeActive()
    {
        return _periodicRxMode;
    }

    void IR_Receiver::SetReceiveTimeout(unsigned long timeoutMs)
    {
        if (timeoutMs >= 100 && timeoutMs <= 10000)
        {
            _timeoutMs = timeoutMs;
        }
        else
        {
#if DRIVERS_DEBUG == 1
            ERR_PRINTLN("[ERR][IR_Receiver] Invalid timeout (100-10000ms)");
#endif
        }
    }

    void IR_Receiver::SetMinPulseWidth(uint16_t minUs)
    {
        _minPulseWidth = minUs;
    }

    void IR_Receiver::SetMaxPulseWidth(uint16_t maxUs)
    {
        _maxPulseWidth = maxUs;
    }

    void IR_Receiver::EnableDebugging(bool enable)
    {
        _debugEnabled = enable;
    }

    IR_Receiver::STATE IR_Receiver::GetCurrentState()
    {
        return _state;
    }

    void IR_Receiver::Update()
    {
        unsigned long currentTime = millis();

        switch (_state)
        {
            case STATE::RECEIVING:
                // Check for receive timeout
                if (currentTime - _stateStartTime > _timeoutMs)
                {
                    _setState(STATE::TIMEOUT);
                    // Timeout is normal when no IR signals are present - no error needed
                }
                // Check if we have received data to process
                else if (_dataReady)
                {
                    _setState(STATE::DECODING);
                    _processReceivedSignal();
                }
                break;

            case STATE::DECODING:
                // Decoding is handled immediately in _processReceivedSignal
                // This state is just for status indication
                break;

            case STATE::SIGNAL_DETECTED:
                // Auto-restart receiving in periodic mode
                if (_periodicRxMode)
                {
                    _setState(STATE::RECEIVING);
                    _pulseIndex = 0;
                    _receiving = true;
                    _dataReady = false;
                }
                else
                {
                    _setState(STATE::IDLE);
                }
                break;

            case STATE::TIMEOUT:
                // Auto-restart receiving in periodic mode
                if (_periodicRxMode)
                {
                    _setState(STATE::RECEIVING);
                    _pulseIndex = 0;
                    _receiving = true;
                    _dataReady = false;
                }
                else
                {
                    _setState(STATE::IDLE);
                }
                break;

            case STATE::IDLE:
                // Auto-start receiving in periodic mode
                if (_periodicRxMode)
                {
                    StartReceiving();
                }
                break;
        }
    }

    // Private Methods - Reception

    void IR_Receiver::_initializeReceiver()
    {
        _pulseIndex = 0;
        _receiving = false;
        _dataReady = false;
        _receivedCode = 0;
        _receivedProtocol = PROTOCOL::UNKNOWN;
        _decodingLength = 0;
    }

    void IR_Receiver::_copyPulseData()
    {
        // Copy volatile buffer to stable buffer for processing
        noInterrupts();
        for (uint16_t i = 0; i < _pulseIndex && i < 128; i++)
        {
            _decodingBuffer[i] = _pulseBuffer[i];
        }
        _decodingLength = _pulseIndex;
        interrupts();
    }

    void IR_Receiver::_processReceivedSignal()
    {
        _copyPulseData();

        bool decoded = false;
        
        // Try different protocols in order of likelihood
        if (_decodeNEC())
        {
            _receivedProtocol = PROTOCOL::NEC;
            decoded = true;
        }
        else if (_decodeSony())
        {
            _receivedProtocol = PROTOCOL::SONY;
            decoded = true;
        }
        else if (_decodeRC5())
        {
            _receivedProtocol = PROTOCOL::RC5;
            decoded = true;
        }

        if (decoded)
        {
            _setState(STATE::SIGNAL_DETECTED);
            _handleSignalReceived();
        }
        else
        {
            // Store as raw data
            _receivedProtocol = PROTOCOL::RAW;
            _receivedCode = 0;  // No code for raw data
            _setState(STATE::SIGNAL_DETECTED);
            _handleSignalReceived();
        }
    }

    bool IR_Receiver::_decodeNEC()
    {
        if (_decodingLength < 34)  // NEC needs at least 34 pulses (start + 32 bits + final)
        {
            return false;
        }

        // Check for NEC start pulse (9ms mark + 4.5ms space)
        if (!_validateNECTiming(_decodingBuffer[0], 9000, 1000) ||
            !_validateNECTiming(_decodingBuffer[1], 4500, 500))
        {
            return false;
        }

        // Decode 32 bits
        uint32_t code = 0;
        for (int i = 0; i < 32; i++)
        {
            uint16_t markIndex = 2 + (i * 2);
            uint16_t spaceIndex = markIndex + 1;
            
            if (spaceIndex >= _decodingLength)
            {
                return false;
            }

            // Check mark timing (should be ~562µs)
            if (!_validateNECTiming(_decodingBuffer[markIndex], 562, 200))
            {
                return false;
            }

            // Check space timing
            // NEC: 562µs space = 0, 1687µs space = 1
            if (_validateNECTiming(_decodingBuffer[spaceIndex], 1687, 300))
            {
                code |= (1UL << (31 - i));  // Set bit (MSB first)
            }
            else if (!_validateNECTiming(_decodingBuffer[spaceIndex], 562, 200))
            {
                return false;  // Invalid space timing
            }
            // else: bit is 0 (already cleared)
        }

        _receivedCode = code;
        return true;
    }

    bool IR_Receiver::_decodeSony()
    {
        if (_decodingLength < 24)  // Sony SIRC-12 needs at least 24 pulses
        {
            return false;
        }

        // Check for Sony start pulse (2.4ms mark + 600µs space)
        if (!_validateSonyTiming(_decodingBuffer[0], 2400, 300) ||
            !_validateSonyTiming(_decodingBuffer[1], 600, 200))
        {
            return false;
        }

        // Decode 12 bits (LSB first for Sony)
        uint16_t code = 0;
        for (int i = 0; i < 12; i++)
        {
            uint16_t markIndex = 2 + (i * 2);
            uint16_t spaceIndex = markIndex + 1;
            
            if (spaceIndex >= _decodingLength)
            {
                break;  // Might be less than 12 bits
            }

            // Sony: 600µs mark + 600µs space = 0, 1200µs mark + 600µs space = 1
            if (_validateSonyTiming(_decodingBuffer[markIndex], 1200, 300))
            {
                code |= (1 << i);  // Set bit (LSB first)
            }
            else if (!_validateSonyTiming(_decodingBuffer[markIndex], 600, 200))
            {
                return false;  // Invalid mark timing
            }

            // Check space timing (should always be ~600µs)
            if (!_validateSonyTiming(_decodingBuffer[spaceIndex], 600, 200))
            {
                return false;
            }
        }

        _receivedCode = code;
        return true;
    }

    bool IR_Receiver::_decodeRC5()
    {
        if (_decodingLength < 28)  // RC5 needs 28 transitions for 14 bits
        {
            return false;
        }

        // RC5 uses Manchester encoding with 1.778ms bit period (889µs half-bit)
        uint16_t bitTime = 889;
        uint16_t code = 0;
        
        // RC5 decoding is more complex due to Manchester encoding
        // This is a simplified version
        for (int i = 0; i < 14 && (i * 2 + 1) < _decodingLength; i++)
        {
            uint16_t firstHalf = _decodingBuffer[i * 2];
            uint16_t secondHalf = _decodingBuffer[i * 2 + 1];
            
            uint8_t bit = _decodeManchesterBit(firstHalf, secondHalf, bitTime);
            if (bit == 2)  // Invalid bit
            {
                return false;
            }
            
            if (bit == 1)
            {
                code |= (1 << (13 - i));  // MSB first
            }
        }

        _receivedCode = code;
        return true;
    }

    void IR_Receiver::_handleSignalReceived()
    {
        if (_rxCallbackEnabled && _rxCallback != nullptr)
        {
            bool isValid = (_state == STATE::SIGNAL_DETECTED);
            _rxCallback(_receivedCode, _receivedProtocol, isValid);
        }
    }

    void IR_Receiver::_handleInterrupt()
    {
        if (!_receiving || _pulseIndex >= 127)
        {
            return;
        }

        unsigned long currentTime = micros();
        
        if (_lastEdgeTime != 0)
        {
            unsigned long duration = currentTime - _lastEdgeTime;
            
            // Filter out noise (too short or too long pulses)
            if (duration >= _minPulseWidth && duration <= _maxPulseWidth)
            {
                _pulseBuffer[_pulseIndex++] = duration;
            }
        }
        
        _lastEdgeTime = currentTime;
        
        // Check if we should stop receiving
        if (_pulseIndex >= 127)
        {
            _receiving = false;
            _dataReady = true;
        }
        
        // Also stop if no edge for a while (end of transmission)
        // This would need a timer check in main Update() function
    }

    void IR_Receiver::_rxInterrupt()
    {
        if (_instance != nullptr)
        {
            _instance->_handleInterrupt();
        }
    }

    // Helper Methods

    void IR_Receiver::_setState(STATE newState)
    {
        _state = newState;
        _stateStartTime = millis();
    }

    void IR_Receiver::_debugPrint(const char* message)
    {
        if (_debugEnabled)
        {
#if DRIVERS_DEBUG == 1
            ERR_PRINT("[DEBUG][IR_Receiver] ");
            ERR_PRINT(message);
#endif
        }
    }

    void IR_Receiver::_debugPrintln(const char* message)
    {
        if (_debugEnabled)
        {
#if DRIVERS_DEBUG == 1
            ERR_PRINT("[DEBUG][IR_Receiver] ");
            ERR_PRINTLN(message);
#endif
        }
    }

    void IR_Receiver::_debugPrintPulses()
    {
        if (_debugEnabled)
        {
#if DRIVERS_DEBUG == 1
            ERR_PRINT("[DEBUG][IR_Receiver] Pulse count: ");
            ERR_PRINTLN(_decodingLength);
            // Could print individual pulse timings if needed
#endif
        }
    }

    bool IR_Receiver::_validateNECTiming(uint16_t timing, uint16_t expected, uint16_t tolerance)
    {
        return (timing >= (expected - tolerance)) && (timing <= (expected + tolerance));
    }

    bool IR_Receiver::_validateSonyTiming(uint16_t timing, uint16_t expected, uint16_t tolerance)
    {
        return (timing >= (expected - tolerance)) && (timing <= (expected + tolerance));
    }

    uint8_t IR_Receiver::_decodeManchesterBit(uint16_t mark, uint16_t space, uint16_t bitTime)
    {
        uint16_t tolerance = bitTime / 4;  // 25% tolerance
        
        // Check if both halves are approximately equal to half bit time
        if (_validateNECTiming(mark, bitTime, tolerance) && 
            _validateNECTiming(space, bitTime, tolerance))
        {
            // In RC5, the second transition determines the bit value
            // This is simplified - actual RC5 needs more complex decoding
            return 1;  // Simplified assumption
        }
        
        return 2;  // Invalid bit
    }

} /* namespace Drivers */
