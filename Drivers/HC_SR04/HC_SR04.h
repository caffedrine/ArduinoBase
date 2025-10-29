#ifndef HC_SR04_H
#define HC_SR04_H

#include "HAL.h"

namespace Drivers
{
    class HC_SR04
    {
    public:
        enum class STATE
        {
            IDLE = 0,
            TRIGGERING = 1,
            WAITING_FOR_ECHO = 2,
            MEASURING = 3,
            COMPLETE = 4,
            TIMEOUT = 5
        };

        // Callback function type for measurement notifications
        // Parameters: distance (cm), isValid (true if measurement successful)
        typedef void (*MeasurementCallback)(float distance, bool isValid);

        HC_SR04(uint8_t triggerPin, uint8_t echoPin);
        ~HC_SR04();

        // Non-blocking measurement
        bool StartMeasurement();           // Start new measurement
        bool IsComplete();                 // Check if measurement done
        float GetDistanceCM();             // Get result in centimeters
        float GetDistanceInches();         // Get result in inches

        // Blocking measurement (for simple use)
        float MeasureDistanceCM();
        float MeasureDistanceInches();

        // Callback-based measurement
        void SetMeasurementCallback(MeasurementCallback callback);
        void ClearMeasurementCallback();
        bool StartMeasurementWithCallback();   // Start measurement with callback notification

        // Periodic measurements
        bool StartPeriodicMeasurements(unsigned long periodMs, MeasurementCallback callback);
        void StopPeriodicMeasurements();
        bool IsPeriodicModeActive();
        unsigned long GetPeriodicPeriod();
        void SetPeriodicPeriod(unsigned long periodMs);

        // Configuration
        void SetTimeout(unsigned long timeoutMs = 30);

        // Call this in main loop for non-blocking operation
        void Update();

    private:
        // Hardware pins
        uint8_t _triggerPin;
        uint8_t _echoPin;

        // State management
        STATE _state;
        unsigned long _startTime;
        unsigned long _echoStart;
        unsigned long _duration;
        float _distance;
        unsigned long _timeoutMs;

        // Callback functionality
        MeasurementCallback _callback;
        bool _callbackEnabled;

        // Periodic measurement
        bool _periodicMode;
        unsigned long _periodicPeriodMs;
        unsigned long _lastPeriodicMeasurement;

        // Helper methods
        void _setState(STATE newState);
        float _calculateDistance(unsigned long duration);
        void _triggerPulse();
        void _handleMeasurementComplete();
        void _handlePeriodicUpdate();
    };

} /* namespace Drivers */

#endif /* HC_SR04_H */
