#ifndef IR_RXLED_H
#define IR_RXLED_H

#include "HAL.h"

namespace Drivers
{
    class IR_Receiver
    {
    public:
        enum class STATE
        {
            IDLE = 0,
            RECEIVING = 1,
            SIGNAL_DETECTED = 2,
            TIMEOUT = 3,
            DECODING = 4
        };

        enum class PROTOCOL
        {
            UNKNOWN = 0,
            NEC = 1,
            RC5 = 2,
            SONY = 3,
            RAW = 4
        };

        // Callback function type for signal reception
        // Parameters: code, protocol, isValid
        typedef void (*SignalReceivedCallback)(uint32_t code, PROTOCOL protocol, bool isValid);

        IR_Receiver(uint8_t rxPin);
        ~IR_Receiver();

        // Reception functions
        bool StartReceiving();
        bool StopReceiving();
        bool IsReceiving();
        bool IsSignalDetected();
        uint32_t GetReceivedCode();
        PROTOCOL GetReceivedProtocol();
        uint16_t* GetRawData(uint16_t* length);  // Get raw pulse timings

        // Callback management
        void SetSignalReceivedCallback(SignalReceivedCallback callback);
        void ClearSignalReceivedCallback();

        // Periodic receiving mode
        bool StartPeriodicReceiving(SignalReceivedCallback callback);
        void StopPeriodicReceiving();
        bool IsPeriodicModeActive();

        // Configuration
        void SetReceiveTimeout(unsigned long timeoutMs = 1000);
        void SetMinPulseWidth(uint16_t minUs = 50);    // Ignore pulses shorter than this
        void SetMaxPulseWidth(uint16_t maxUs = 10000); // Ignore pulses longer than this

        // Call this in main loop for non-blocking operation
        void Update();

        // Utility functions
        void EnableDebugging(bool enable = true);
        STATE GetCurrentState();

    private:
        // Hardware pin
        uint8_t _rxPin;

        // State management
        STATE _state;
        unsigned long _stateStartTime;
        unsigned long _timeoutMs;

        // Reception variables
        volatile unsigned long _lastEdgeTime;
        volatile uint16_t _pulseBuffer[128];  // Buffer for pulse timings
        volatile uint16_t _pulseIndex;
        volatile bool _receiving;
        volatile bool _dataReady;
        
        // Decoding variables
        uint32_t _receivedCode;
        PROTOCOL _receivedProtocol;
        uint16_t _decodingBuffer[128];
        uint16_t _decodingLength;
        uint16_t _minPulseWidth;
        uint16_t _maxPulseWidth;

        // Callback functionality
        SignalReceivedCallback _rxCallback;
        bool _rxCallbackEnabled;
        bool _periodicRxMode;

        // Debug
        bool _debugEnabled;

        // Helper methods - Reception
        void _initializeReceiver();
        void _processReceivedSignal();
        bool _decodeNEC();
        bool _decodeRC5();
        bool _decodeSony();
        void _handleSignalReceived();
        void _copyPulseData();

        // Interrupt handling
        void _handleInterrupt();
        static void _rxInterrupt();
        static IR_Receiver* _instance;

        // Helper methods - General
        void _setState(STATE newState);
        void _debugPrint(const char* message);
        void _debugPrintln(const char* message);
        void _debugPrintPulses();

        // Protocol-specific decoding helpers
        bool _validateNECTiming(uint16_t timing, uint16_t expected, uint16_t tolerance = 200);
        bool _validateSonyTiming(uint16_t timing, uint16_t expected, uint16_t tolerance = 200);
        uint8_t _decodeManchesterBit(uint16_t mark, uint16_t space, uint16_t bitTime);
    };

} /* namespace Drivers */

#endif /* IR_RXLED_H */