#ifndef IR_TXLED_H
#define IR_TXLED_H

#include "HAL.h"

namespace Drivers
{
    class IR_TxLed
    {
    public:
        enum class STATE
        {
            IDLE = 0,
            TRANSMITTING = 1,
            COMPLETE = 2,
            ERROR = 3
        };

        enum class PROTOCOL
        {
            NEC = 0,
            RC5 = 1,
            SONY = 2,
            RAW = 3
        };

        // Callback function type for transmission completion
        typedef void (*TransmissionCompleteCallback)(bool success);

        IR_TxLed(uint8_t txPin);
        ~IR_TxLed();

        // Transmission functions
        bool TransmitNEC(uint32_t code);
        bool TransmitRC5(uint16_t code);
        bool TransmitSony(uint16_t code);
        bool TransmitRaw(uint16_t* timings, uint16_t length);
        bool IsTransmitting();
        bool IsComplete();

        // Callback management
        void SetTransmissionCompleteCallback(TransmissionCompleteCallback callback);
        void ClearTransmissionCompleteCallback();

        // Configuration
        void SetCarrierFrequency(uint16_t frequency = 38000);  // 38kHz default
        void SetTransmitPower(uint8_t power = 255);  // 0-255 PWM value
        void SetRepeatCount(uint8_t repeats = 0);    // Number of repeats

        // Call this in main loop for non-blocking operation
        void Update();

        // Utility functions
        void EnableDebugging(bool enable = true);
        STATE GetCurrentState();

    private:
        // Hardware pin
        uint8_t _txPin;

        // State management
        STATE _state;
        unsigned long _stateStartTime;

        // Transmission variables
        uint16_t _carrierFreq;
        uint8_t _transmitPower;
        uint16_t* _transmitBuffer;
        uint16_t _transmitLength;
        uint16_t _transmitIndex;
        unsigned long _lastTransmitToggle;
        bool _transmitCarrierState;
        uint8_t _repeatCount;
        uint8_t _currentRepeat;
        
        // Callback
        TransmissionCompleteCallback _txCallback;
        bool _txCallbackEnabled;

        // Debug
        bool _debugEnabled;

        // Helper methods - Transmission
        void _startTransmission(uint16_t* timings, uint16_t length);
        void _updateTransmission();
        void _completeTransmission(bool success);
        void _generateCarrierPulse(unsigned long durationUs);

        // Helper methods - General
        void _setState(STATE newState);
        void _debugPrint(const char* message);
        void _debugPrintln(const char* message);

        // Protocol generation methods
        void _generateNECCode(uint32_t code, uint16_t* buffer, uint16_t* length);
        void _generateRC5Code(uint16_t code, uint16_t* buffer, uint16_t* length);
        void _generateSonyCode(uint16_t code, uint16_t* buffer, uint16_t* length);
    };

} /* namespace Drivers */

#endif /* IR_TXLED_H */
