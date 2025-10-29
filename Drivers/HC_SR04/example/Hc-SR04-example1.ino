/*
 * HC-SR04 Ultrasonic Sensor Usage Examples
 * Simple, non-blocking distance measurement
 */

#include "HC_SR04.h"
// or #include "HC_SR04_Simple.h" if using HAL version

using namespace Drivers;

// Create sensor instance
HC_SR04_Arduino sensor(7, 8);  // Trigger pin 7, Echo pin 8

void setup() {
    Serial.begin(9600);
    Serial.println("HC-SR04 Ultrasonic Sensor Demo");
    
    // Configure sensor (optional)
    sensor.SetTimeout(50);        // 50ms timeout
    sensor.SetMinInterval(100);   // Minimum 100ms between measurements
    
    // Test blocking measurement
    Serial.println("\n=== Blocking Measurement Test ===");
    float distance = sensor.MeasureDistanceCM();
    if (distance > 0) {
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.print(" cm (");
        Serial.print(distance / 2.54);
        Serial.println(" inches)");
    } else {
        Serial.println("Measurement failed!");
    }
    
    delay(1000);
    
    // Start continuous non-blocking measurements
    Serial.println("\n=== Non-blocking Continuous Measurements ===");
    sensor.StartMeasurement();
}

void loop() {
    // Update sensor state machine (CRITICAL!)
    sensor.Update();
    
    // Check if measurement is complete
    if (sensor.IsComplete()) {
        float distance = sensor.GetDistanceCM();
        
        if (distance > 0) {
            Serial.print("Distance: ");
            Serial.print(distance, 1);
            Serial.println(" cm");
            
            // Distance-based actions
            if (distance < 10) {
                Serial.println("  -> Object very close!");
            } else if (distance < 30) {
                Serial.println("  -> Object nearby");
            } else if (distance > 200) {
                Serial.println("  -> Object far away");
            }
        } else {
            Serial.println("Measurement failed or out of range");
        }
        
        // Start next measurement
        delay(200); // Wait a bit between measurements
        sensor.StartMeasurement();
    }
    
    // Your other main loop code here
    // The sensor runs non-blocking, so this won't interfere
    doOtherWork();
}

void doOtherWork() {
    // Simulate other work in main loop
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 1000) {
        digitalWrite(13, !digitalRead(13)); // Blink built-in LED
        lastBlink = millis();
    }
}

/*
 * Advanced Usage Examples
 */

void advancedExamples() {
    // State checking
    switch (sensor.GetState()) {
        case HC_SR04_Arduino::STATE::IDLE:
            Serial.println("Sensor idle");
            break;
        case HC_SR04_Arduino::STATE::MEASURING:
            Serial.println("Measurement in progress...");
            break;
        case HC_SR04_Arduino::STATE::COMPLETE:
            Serial.println("Measurement complete");
            break;
        case HC_SR04_Arduino::STATE::TIMEOUT:
            Serial.println("Measurement timed out");
            break;
    }
    
    // Check if ready for new measurement
    if (sensor.IsReady()) {
        sensor.StartMeasurement();
    }
    
    // Get distance in different units
    if (sensor.IsComplete()) {
        float cm = sensor.GetDistanceCM();
        float inches = sensor.GetDistanceInches();
        
        Serial.print("Distance: ");
        Serial.print(cm);
        Serial.print(" cm = ");
        Serial.print(inches);
        Serial.println(" inches");
    }
}

/*
 * Multiple Sensors Example
 */

HC_SR04_Arduino frontSensor(7, 8);   // Front sensor
HC_SR04_Arduino backSensor(9, 10);   // Back sensor
HC_SR04_Arduino leftSensor(11, 12);  // Left sensor

void multipleSensorsExample() {
    // Update all sensors
    frontSensor.Update();
    backSensor.Update();
    leftSensor.Update();
    
    // Start measurements in sequence to avoid interference
    static unsigned long lastMeasurement = 0;
    static uint8_t currentSensor = 0;
    
    if (millis() - lastMeasurement > 100) { // 100ms between sensor activations
        switch (currentSensor) {
            case 0:
                if (frontSensor.IsReady()) {
                    frontSensor.StartMeasurement();
                    currentSensor = 1;
                    lastMeasurement = millis();
                }
                break;
            case 1:
                if (backSensor.IsReady()) {
                    backSensor.StartMeasurement();
                    currentSensor = 2;
                    lastMeasurement = millis();
                }
                break;
            case 2:
                if (leftSensor.IsReady()) {
                    leftSensor.StartMeasurement();
                    currentSensor = 0;
                    lastMeasurement = millis();
                }
                break;
        }
    }
    
    // Process results
    if (frontSensor.IsComplete()) {
        Serial.print("Front: ");
        Serial.println(frontSensor.GetDistanceCM());
    }
    if (backSensor.IsComplete()) {
        Serial.print("Back: ");
        Serial.println(backSensor.GetDistanceCM());
    }
    if (leftSensor.IsComplete()) {
        Serial.print("Left: ");
        Serial.println(leftSensor.GetDistanceCM());
    }
}

/*
 * Robot Obstacle Avoidance Example
 */

void obstacleAvoidanceExample() {
    sensor.Update();
    
    static unsigned long lastCheck = 0;
    
    // Check distance every 200ms
    if (millis() - lastCheck > 200) {
        if (sensor.IsReady()) {
            sensor.StartMeasurement();
            lastCheck = millis();
        }
    }
    
    // React to measurements
    if (sensor.IsComplete()) {
        float distance = sensor.GetDistanceCM();
        
        if (distance > 0) {
            if (distance < 15) {
                Serial.println("STOP! Obstacle detected!");
                // Stop motors, turn, etc.
            } else if (distance < 30) {
                Serial.println("Slow down - obstacle ahead");
                // Reduce speed
            } else {
                Serial.println("Path clear");
                // Normal speed
            }
        }
    }
}

/*
 * Tips for reliable operation:
 * 
 * 1. Always call sensor.Update() in your main loop
 * 2. Don't start new measurements too frequently (min 60ms apart)
 * 3. Handle timeout cases gracefully
 * 4. For multiple sensors, stagger measurements to avoid interference
 * 5. HC-SR04 range: 2cm to 400cm (reliable range: 10cm to 300cm)
 * 6. Avoid obstacles at steep angles (>15 degrees from perpendicular)
 * 7. Works best with flat, solid surfaces
 */