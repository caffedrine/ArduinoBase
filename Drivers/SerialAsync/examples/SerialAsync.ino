/*
 * HC-SR04 Enhanced Usage Examples
 * Demonstrates callback functionality and periodic measurements
 */

#include "HC_SR04.h"
// #include "HC_SR04_Modern.h"  // For lambda support

using namespace Drivers;

// Global sensor instance
HC_SR04 sensor(7, 8);  // Trigger pin 7, Echo pin 8

// Global variables for examples
float lastDistance = 0;
unsigned long measurementCount = 0;

/*
 * Example 1: Basic Callback Usage
 */

void onMeasurementComplete(float distance, bool isValid) {
    measurementCount++;

    if (isValid) {
        lastDistance = distance;
        Serial.print("Measurement #");
        Serial.print(measurementCount);
        Serial.print(": ");
        Serial.print(distance, 1);
        Serial.println(" cm");

        // Distance-based actions
        if (distance < 10) {
            Serial.println("  -> ALERT: Object very close!");
            // Could trigger LED, buzzer, etc.
        } else if (distance < 30) {
            Serial.println("  -> Warning: Object nearby");
        }
    } else {
        Serial.print("Measurement #");
        Serial.print(measurementCount);
        Serial.println(": FAILED (timeout or error)");
    }
}

void basicCallbackExample() {
    Serial.println("=== Basic Callback Example ===");

    // Set up callback
    sensor.SetMeasurementCallback(onMeasurementComplete);

    // Start single measurement with callback
    sensor.StartMeasurementWithCallback();

    // Main loop simulation
    for (int i = 0; i < 100; i++) {
        sensor.Update();  // Process measurement
        delay(10);        // Simulate other work

        if (sensor.IsComplete()) {
            break;  // Measurement finished
        }
    }

    // Clear callback when done
    sensor.ClearMeasurementCallback();
}

/*
 * Example 2: Periodic Measurements
 */

void onPeriodicMeasurement(float distance, bool isValid) {
    static unsigned long lastPrint = 0;

    if (millis() - lastPrint > 1000) {  // Print every second
        if (isValid) {
            Serial.print("Periodic: ");
            Serial.print(distance, 1);
            Serial.println(" cm");
        } else {
            Serial.println("Periodic: FAILED");
        }
        lastPrint = millis();
    }
}

void periodicMeasurementExample() {
    Serial.println("\n=== Periodic Measurement Example ===");

    // Start periodic measurements every 200ms
    if (sensor.StartPeriodicMeasurements(200, onPeriodicMeasurement)) {
        Serial.println("Started periodic measurements (200ms interval)");

        // Let it run for 10 seconds
        unsigned long startTime = millis();
        while (millis() - startTime < 10000) {
            sensor.Update();  // CRITICAL: Must call this!

            // Simulate other work
            delay(50);
        }

        // Stop periodic measurements
        sensor.StopPeriodicMeasurements();
        Serial.println("Stopped periodic measurements");
    } else {
        Serial.println("Failed to start periodic measurements");
    }
}

/*
 * Example 3: Distance-Based State Machine
 */

enum RobotState {
    MOVING_FORWARD,
    SLOWING_DOWN,
    STOPPING,
    BACKING_UP
};

RobotState currentState = MOVING_FORWARD;

void onRobotDistanceMeasurement(float distance, bool isValid) {
    if (!isValid) {
        return;  // Ignore failed measurements
    }

    // State machine based on distance
    switch (currentState) {
        case MOVING_FORWARD:
            if (distance < 30) {
                currentState = SLOWING_DOWN;
                Serial.println("Robot: Slowing down (obstacle at 30cm)");
            }
            break;

        case SLOWING_DOWN:
            if (distance < 15) {
                currentState = STOPPING;
                Serial.println("Robot: STOPPING (obstacle at 15cm)");
            } else if (distance > 50) {
                currentState = MOVING_FORWARD;
                Serial.println("Robot: Resuming normal speed");
            }
            break;

        case STOPPING:
            if (distance < 10) {
                currentState = BACKING_UP;
                Serial.println("Robot: BACKING UP (obstacle too close)");
            } else if (distance > 30) {
                currentState = MOVING_FORWARD;
                Serial.println("Robot: Path clear, moving forward");
            }
            break;

        case BACKING_UP:
            if (distance > 25) {
                currentState = MOVING_FORWARD;
                Serial.println("Robot: Safe distance, moving forward");
            }
            break;
    }
}

void robotStateMachineExample() {
    Serial.println("\n=== Robot State Machine Example ===");

    // Start robot with periodic distance monitoring
    sensor.StartPeriodicMeasurements(150, onRobotDistanceMeasurement);
    Serial.println("Robot navigation started");

    // Simulate robot operation for 15 seconds
    unsigned long startTime = millis();
    while (millis() - startTime < 15000) {
        sensor.Update();

        // Simulate robot control based on current state
        switch (currentState) {
            case MOVING_FORWARD:
                // Set motors to normal speed
                break;
            case SLOWING_DOWN:
                // Reduce motor speed
                break;
            case STOPPING:
                // Stop motors
                break;
            case BACKING_UP:
                // Reverse motors
                break;
        }

        delay(100);  // Simulate control loop timing
    }

    sensor.StopPeriodicMeasurements();
    Serial.println("Robot navigation stopped");
}

/*
 * Example 4: Multi-Sensor Array with Callbacks
 */

HC_SR04 frontSensor(7, 8);
HC_SR04 leftSensor(9, 10);
HC_SR04 rightSensor(11, 12);

struct SensorData {
    float frontDistance = -1;
    float leftDistance = -1;
    float rightDistance = -1;
    unsigned long lastUpdate = 0;
};

SensorData sensorArray;

void onFrontSensorMeasurement(float distance, bool isValid) {
    sensorArray.frontDistance = isValid ? distance : -1;
    sensorArray.lastUpdate = millis();
}

void onLeftSensorMeasurement(float distance, bool isValid) {
    sensorArray.leftDistance = isValid ? distance : -1;
    sensorArray.lastUpdate = millis();
}

void onRightSensorMeasurement(float distance, bool isValid) {
    sensorArray.rightDistance = isValid ? distance : -1;
    sensorArray.lastUpdate = millis();
}

void multiSensorExample() {
    Serial.println("\n=== Multi-Sensor Array Example ===");

    // Set up callbacks for each sensor
    frontSensor.SetMeasurementCallback(onFrontSensorMeasurement);
    leftSensor.SetMeasurementCallback(onLeftSensorMeasurement);
    rightSensor.SetMeasurementCallback(onRightSensorMeasurement);

    // Start periodic measurements with staggered timing to avoid interference
    frontSensor.StartPeriodicMeasurements(300, onFrontSensorMeasurement);
    delay(100);  // Offset timing
    leftSensor.StartPeriodicMeasurements(300, onLeftSensorMeasurement);
    delay(100);  // Offset timing
    rightSensor.StartPeriodicMeasurements(300, onRightSensorMeasurement);

    Serial.println("Multi-sensor array started");

    // Monitor all sensors for 10 seconds
    unsigned long startTime = millis();
    unsigned long lastPrint = 0;

    while (millis() - startTime < 10000) {
        // Update all sensors
        frontSensor.Update();
        leftSensor.Update();
        rightSensor.Update();

        // Print sensor data every 2 seconds
        if (millis() - lastPrint > 2000) {
            Serial.print("Sensors -> Front: ");
            Serial.print(sensorArray.frontDistance, 1);
            Serial.print("cm, Left: ");
            Serial.print(sensorArray.leftDistance, 1);
            Serial.print("cm, Right: ");
            Serial.print(sensorArray.rightDistance, 1);
            Serial.println("cm");

            lastPrint = millis();
        }

        delay(50);
    }

    // Stop all sensors
    frontSensor.StopPeriodicMeasurements();
    leftSensor.StopPeriodicMeasurements();
    rightSensor.StopPeriodicMeasurements();

    Serial.println("Multi-sensor array stopped");
}

/*
 * Arduino Setup and Loop
 */

void setup() {
    Serial.begin(9600);
    Serial.println("HC-SR04 Enhanced Examples");
    Serial.println("=========================");

    // Configure sensor
    sensor.SetTimeout(50);  // 50ms timeout

    // Run examples
    basicCallbackExample();
    delay(2000);

    periodicMeasurementExample();
    delay(2000);

    robotStateMachineExample();
    delay(2000);

    multiSensorExample();

    Serial.println("\nAll examples completed!");
}

void loop() {
    // Main loop can be empty - all work is done via callbacks
    delay(1000);
}

/*
 * Lambda Examples (requires HC_SR04_Modern class)
 */

#ifdef HC_SR04_MODERN_H

void lambdaExamples() {
    HC_SR04_Modern modernSensor(7, 8);

    // Example 1: Simple lambda
    modernSensor.SetMeasurementCallback([](float distance, bool isValid) {
        if (isValid) {
            Serial.print("Lambda: ");
            Serial.println(distance);
        }
    });

    // Example 2: Lambda with capture
    float threshold = 20.0;
    int alertCount = 0;

    modernSensor.SetMeasurementCallback([&threshold, &alertCount](float distance, bool isValid) {
        if (isValid && distance < threshold) {
            alertCount++;
            Serial.print("ALERT #");
            Serial.print(alertCount);
            Serial.print(": Distance ");
            Serial.print(distance);
            Serial.print(" < ");
            Serial.println(threshold);
        }
    });

    // Example 3: Periodic with lambda
    modernSensor.StartPeriodicMeasurements(200, [](float distance, bool isValid) {
        static unsigned long count = 0;
        count++;

        if (isValid) {
            Serial.print("Reading #");
            Serial.print(count);
            Serial.print(": ");
            Serial.println(distance);
        }
    });
}

#endif

/*
 * Usage Tips:
 *
 * 1. Always call sensor.Update() in your main loop
 * 2. Callbacks are called from Update(), so keep them fast
 * 3. For multiple sensors, stagger periodic measurements to avoid interference
 * 4. Minimum recommended period is 60ms for reliable operation
 * 5. Use callbacks for real-time response to distance changes
 * 6. Periodic mode is perfect for continuous monitoring applications
 */
