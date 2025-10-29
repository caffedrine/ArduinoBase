#include "HC_SR04.h"

namespace Drivers
{

HC_SR04::HC_SR04(uint8_t triggerPin, uint8_t echoPin):
    _triggerPin(triggerPin), _echoPin(echoPin), _timeoutMs(30),
    _callback(nullptr), _callbackEnabled(false),
    _periodicMode(false), _periodicPeriodMs(0), _lastPeriodicMeasurement(0)
{
    // Initialize pins
    Vfb_SetPinMode(_triggerPin, OUTPUT);
    Vfb_SetPinMode(_echoPin, INPUT);

    // Ensure trigger starts low
    Vfb_DigitalWrite(_triggerPin, LOW);

    // Initialize state
    _setState(STATE::IDLE);
    _distance = 0.0;
    _duration = 0;

    #if DRIVERS_DEBUG == 1
        ERR_PRINT("[INFO][HC_SR04] Initialized: Trigger=");
        ERR_PRINT(_triggerPin);
        ERR_PRINT(", Echo=");
        ERR_PRINTLN(_echoPin);
    #endif
}

HC_SR04::~HC_SR04()
{
    // Stop any periodic measurements
    StopPeriodicMeasurements();

    // Set trigger pin to safe state
    Vfb_DigitalWrite(_triggerPin, LOW);
}

bool HC_SR04::StartMeasurement()
{
    if (_state != STATE::IDLE && _state != STATE::COMPLETE && _state != STATE::TIMEOUT) {
        return false; // Measurement already in progress
    }

    _setState(STATE::TRIGGERING);
    _startTime = millis();
    _triggerPulse();

    return true;
}

bool HC_SR04::IsComplete()
{
    return (_state == STATE::COMPLETE || _state == STATE::TIMEOUT);
}

float HC_SR04::GetDistanceCM()
{
    if (_state == STATE::COMPLETE) {
        return _distance;
    }
    return -1.0; // Invalid measurement
}

float HC_SR04::GetDistanceInches()
{
    float cm = GetDistanceCM();
    if (cm >= 0) {
        return cm / 2.54; // Convert CM to inches
    }
    return -1.0;
}

float HC_SR04::MeasureDistanceCM()
{
    if (!StartMeasurement()) {
        return -1.0;
    }

    // Wait for completion
    while (!IsComplete()) {
        Update();
        if (millis() - _startTime > _timeoutMs + 10) {
            break; // Safety timeout
        }
    }

    return GetDistanceCM();
}

float HC_SR04::MeasureDistanceInches()
{
    float cm = MeasureDistanceCM();
    if (cm >= 0) {
        return cm / 2.54;
    }
    return -1.0;
}

void HC_SR04::SetMeasurementCallback(MeasurementCallback callback)
{
    _callback = callback;
    _callbackEnabled = (callback != nullptr);

    #if DRIVERS_DEBUG == 1
        if (_callbackEnabled) {
            ERR_PRINTLN("[INFO][HC_SR04] Measurement callback enabled");
        } else {
            ERR_PRINTLN("[INFO][HC_SR04] Measurement callback disabled");
        }
    #endif
}

void HC_SR04::ClearMeasurementCallback()
{
    _callback = nullptr;
    _callbackEnabled = false;

    #if DRIVERS_DEBUG == 1
        ERR_PRINTLN("[INFO][HC_SR04] Measurement callback cleared");
    #endif
}

bool HC_SR04::StartMeasurementWithCallback()
{
    if (!_callbackEnabled) {
        #if DRIVERS_DEBUG == 1
            ERR_PRINTLN("[ERR][HC_SR04] No callback set for StartMeasurementWithCallback");
        #endif
        return false;
    }

    return StartMeasurement();
}

bool HC_SR04::StartPeriodicMeasurements(unsigned long periodMs, MeasurementCallback callback)
{
    if (periodMs < 60) {
        #if DRIVERS_DEBUG == 1
            ERR_PRINTLN("[ERR][HC_SR04] Periodic period too short (min 60ms)");
        #endif
        return false;
    }

    // Set callback
    SetMeasurementCallback(callback);

    // Configure periodic mode
    _periodicMode = true;
    _periodicPeriodMs = periodMs;
    _lastPeriodicMeasurement = millis();

    #if DRIVERS_DEBUG == 1
        ERR_PRINT("[INFO][HC_SR04] Started periodic measurements: ");
        ERR_PRINT(_periodicPeriodMs);
        ERR_PRINTLN("ms");
    #endif

    // Start first measurement immediately
    return StartMeasurement();
}

void HC_SR04::StopPeriodicMeasurements()
{
    _periodicMode = false;
    _periodicPeriodMs = 0;

    #if DRIVERS_DEBUG == 1
        ERR_PRINTLN("[INFO][HC_SR04] Stopped periodic measurements");
    #endif
}

bool HC_SR04::IsPeriodicModeActive()
{
    return _periodicMode;
}

unsigned long HC_SR04::GetPeriodicPeriod()
{
    return _periodicPeriodMs;
}

void HC_SR04::SetPeriodicPeriod(unsigned long periodMs)
{
    if (periodMs >= 60) {
        _periodicPeriodMs = periodMs;

        #if DRIVERS_DEBUG == 1
            ERR_PRINT("[INFO][HC_SR04] Updated periodic period: ");
            ERR_PRINT(_periodicPeriodMs);
            ERR_PRINTLN("ms");
        #endif
    } else {
        #if DRIVERS_DEBUG == 1
            ERR_PRINTLN("[ERR][HC_SR04] Periodic period too short (min 60ms)");
        #endif
    }
}

void HC_SR04::SetTimeout(unsigned long timeoutMs)
{
    _timeoutMs = timeoutMs;
}

void HC_SR04::Update()
{
    unsigned long currentTime = millis();

    switch (_state) {
        case STATE::TRIGGERING:
            // Wait for trigger pulse to complete (10us)
            if (micros() - _startTime >= 10) {
                Vfb_DigitalWrite(_triggerPin, LOW);
                _setState(STATE::WAITING_FOR_ECHO);
            }
            break;

        case STATE::WAITING_FOR_ECHO:
            // Wait for echo pin to go HIGH
            if (Vfb_DigitalRead(_echoPin) == HIGH) {
                _echoStart = micros();
                _setState(STATE::MEASURING);
            } else if (currentTime - _startTime > _timeoutMs) {
                _setState(STATE::TIMEOUT);
                _handleMeasurementComplete(); // Handle timeout with callback
                #if DRIVERS_DEBUG == 1
                    ERR_PRINTLN("[ERR][HC_SR04] Timeout waiting for echo start");
                #endif
            }
            break;

        case STATE::MEASURING:
            // Wait for echo pin to go LOW
            if (Vfb_DigitalRead(_echoPin) == LOW) {
                _duration = micros() - _echoStart;
                _distance = _calculateDistance(_duration);
                _setState(STATE::COMPLETE);
                _handleMeasurementComplete(); // Handle successful measurement
            } else if (currentTime - _startTime > _timeoutMs) {
                _setState(STATE::TIMEOUT);
                _handleMeasurementComplete(); // Handle timeout with callback
//                #if DRIVERS_DEBUG == 1
//                    ERR_PRINTLN("[ERR][HC_SR04] Timeout during measurement");
//                #endif
            }
            break;

        case STATE::IDLE:
        case STATE::COMPLETE:
        case STATE::TIMEOUT:
            // Handle periodic measurements
            if (_periodicMode) {
                _handlePeriodicUpdate();
            }
            break;
    }
}

// Private methods

void HC_SR04::_setState(STATE newState)
{
    _state = newState;
}

float HC_SR04::_calculateDistance(unsigned long duration)
{
    // Speed of sound = 343 m/s = 34300 cm/s
    // Distance = (duration * speed) / 2 (round trip)
    // Distance = (duration in microseconds * 34300 cm/s) / (2 * 1000000 us/s)
    // Distance = duration * 0.01715

    return (float)duration * 0.01715;
}

void HC_SR04::_triggerPulse()
{
    // Generate 10us trigger pulse
    Vfb_DigitalWrite(_triggerPin, HIGH);
    // Note: Trigger will be turned off in Update() after 10us
}

void HC_SR04::_handleMeasurementComplete()
{
    // Call callback if enabled
    if (_callbackEnabled && _callback != nullptr) {
        bool isValid = (_state == STATE::COMPLETE);
        float distance = isValid ? _distance : -1.0;

        // Call the callback function
        _callback(distance, isValid);
    }
}

void HC_SR04::_handlePeriodicUpdate()
{
    if (!_periodicMode) {
        return;
    }

    unsigned long currentTime = millis();

    // Check if it's time for next periodic measurement
    if (currentTime - _lastPeriodicMeasurement >= _periodicPeriodMs) {
        // Only start new measurement if sensor is ready
        if (_state == STATE::IDLE || _state == STATE::COMPLETE || _state == STATE::TIMEOUT) {
            _lastPeriodicMeasurement = currentTime;
            StartMeasurement();
        }
    }
}

} /* namespace Drivers */
