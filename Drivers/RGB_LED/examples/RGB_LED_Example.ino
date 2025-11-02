#include "RGB_LED.h"

// Create RGB LED instance with pins 9, 10, 11 (must be PWM capable pins)
Drivers::RGB_LED rgb_led(9, 10, 11);

void setup()
{
    Serial.begin(115200);
    Serial.println("RGB LED Example Started");
    
    // Example 1: Set solid color (Red)
    rgb_led.SetColor(255, 0, 0);
    delay(2000);
    
    // Example 2: Fade to Green over 1 second
    rgb_led.FadeTo(0, 255, 0, 1000);
}

void loop()
{
    // IMPORTANT: Call Update() cyclically for non-blocking operation
    rgb_led.Update();
    
    // Your other code can run here without blocking
    // The LED effects will continue to update in the background
}