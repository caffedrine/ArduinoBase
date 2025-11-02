#ifndef RGB_LED_H
#define RGB_LED_H

#include "HAL.h"

namespace Drivers
{
    class RGB_LED
    {
    public:
        enum class STATE
        {
            OFF = 0,
            SOLID = 1,
            FADING = 2,
            BLINKING = 3,
            BREATHING = 4
        };

        enum class BLINK_MODE
        {
            NONE = 0,
            CONTINUOUS = 1,
            COUNT_LIMITED = 2
        };

        /**
         * @brief Constructor - initializes RGB LED with PWM pins
         * @param pwm_pin_R Red channel PWM pin
         * @param pwm_pin_G Green channel PWM pin
         * @param pwm_pin_B Blue channel PWM pin
         */
        RGB_LED(uint8_t pwm_pin_R, uint8_t pwm_pin_G, uint8_t pwm_pin_B);

        /**
         * @brief Destructor - turns off LED
         */
        ~RGB_LED();

        /**
         * @brief Must be called cyclically in main loop for non-blocking operation
         */
        void Update();

        /**
         * @brief Set LED to a solid color immediately
         * @param r Red value (0-255)
         * @param g Green value (0-255)
         * @param b Blue value (0-255)
         */
        void SetColor(uint8_t r, uint8_t g, uint8_t b);

        /**
         * @brief Fade LED to target color over specified duration
         * @param r Target red value (0-255)
         * @param g Target green value (0-255)
         * @param b Target blue value (0-255)
         * @param duration_ms Fade duration in milliseconds
         */
        void FadeTo(uint8_t r, uint8_t g, uint8_t b, uint16_t duration_ms = 1000);

        /**
         * @brief Start blinking LED
         * @param r Red value (0-255)
         * @param g Green value (0-255)
         * @param b Blue value (0-255)
         * @param on_time_ms Time LED is on in milliseconds
         * @param off_time_ms Time LED is off in milliseconds
         * @param mode CONTINUOUS or COUNT_LIMITED
         * @param count Number of blinks (only used if mode is COUNT_LIMITED)
         */
        void StartBlinking(uint8_t r, uint8_t g, uint8_t b,
                          uint16_t on_time_ms = 500,
                          uint16_t off_time_ms = 500,
                          BLINK_MODE mode = BLINK_MODE::CONTINUOUS,
                          uint16_t count = 0);

        /**
         * @brief Start breathing effect (smooth fade in/out)
         * @param r Red value (0-255)
         * @param g Green value (0-255)
         * @param b Blue value (0-255)
         * @param period_ms Complete breathing cycle duration in milliseconds
         */
        void StartBreathing(uint8_t r, uint8_t g, uint8_t b, uint16_t period_ms = 2000);

        /**
         * @brief Stop blinking and maintain current state
         */
        void StopBlinking();

        /**
         * @brief Turn off LED
         */
        void Off();

        /**
         * @brief Get current LED state
         * @return Current STATE
         */
        STATE GetState() const;

        /**
         * @brief Get current color values
         * @param r Reference to store red value
         * @param g Reference to store green value
         * @param b Reference to store blue value
         */
        void GetCurrentColor(uint8_t& r, uint8_t& g, uint8_t& b) const;

    private:
        // Pin assignments
        uint8_t m_pin_R;
        uint8_t m_pin_G;
        uint8_t m_pin_B;

        // State management
        STATE m_state;
        BLINK_MODE m_blink_mode;

        // Current and target color values
        uint8_t m_current_R;
        uint8_t m_current_G;
        uint8_t m_current_B;
        uint8_t m_target_R;
        uint8_t m_target_G;
        uint8_t m_target_B;

        // Blink parameters
        uint16_t m_blink_count;
        uint16_t m_blink_max_count;
        uint16_t m_blink_on_time;
        uint16_t m_blink_off_time;
        bool m_is_on;

        // Fade/breathing parameters
        uint16_t m_fade_duration;
        float m_breathing_phase;

        // Timing
        unsigned long m_last_update;
        unsigned long m_blink_timer;

        // Private helper methods
        void UpdateFading(unsigned long current_time);
        void UpdateBlinking(unsigned long current_time);
        void UpdateBreathing(unsigned long current_time);
        void WriteToHardware();
    };

} /* namespace Drivers */

#endif /* RGB_LED_H */
