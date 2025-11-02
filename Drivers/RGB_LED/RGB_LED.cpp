#include "RGB_LED.h"

namespace Drivers
{
    RGB_LED::RGB_LED(uint8_t pwm_pin_R, uint8_t pwm_pin_G, uint8_t pwm_pin_B)
        : m_pin_R(pwm_pin_R),
          m_pin_G(pwm_pin_G),
          m_pin_B(pwm_pin_B),
          m_state(STATE::OFF),
          m_blink_mode(BLINK_MODE::NONE),
          m_current_R(0),
          m_current_G(0),
          m_current_B(0),
          m_target_R(0),
          m_target_G(0),
          m_target_B(0),
          m_blink_count(0),
          m_blink_max_count(0),
          m_blink_on_time(500),
          m_blink_off_time(500),
          m_fade_duration(1000),
          m_last_update(0),
          m_blink_timer(0),
          m_is_on(false),
          m_breathing_phase(0)
    {
        Vfb_SetPinMode(m_pin_R, OUTPUT);
        Vfb_SetPinMode(m_pin_G, OUTPUT);
        Vfb_SetPinMode(m_pin_B, OUTPUT);

        // Initialize to off
        SetColor(0, 0, 0);
    }

    RGB_LED::~RGB_LED()
    {
        // Turn off LED
        SetColor(0, 0, 0);
    }

    void RGB_LED::Update()
    {
        unsigned long current_time = millis();

        switch (m_state)
        {
            case STATE::OFF:
                // Do nothing, LED is off
                break;

            case STATE::SOLID:
                // Static color, no updates needed after initial set
                break;

            case STATE::FADING:
                UpdateFading(current_time);
                break;

            case STATE::BLINKING:
                UpdateBlinking(current_time);
                break;

            case STATE::BREATHING:
                UpdateBreathing(current_time);
                break;
        }
    }

    void RGB_LED::SetColor(uint8_t r, uint8_t g, uint8_t b)
    {
        m_current_R = r;
        m_current_G = g;
        m_current_B = b;

        m_target_R = r;
        m_target_G = g;
        m_target_B = b;

        WriteToHardware();

        if (r == 0 && g == 0 && b == 0)
        {
            m_state = STATE::OFF;
        }
        else
        {
            m_state = STATE::SOLID;
        }

        m_blink_mode = BLINK_MODE::NONE;
    }

    void RGB_LED::FadeTo(uint8_t r, uint8_t g, uint8_t b, uint16_t duration_ms)
    {
        m_target_R = r;
        m_target_G = g;
        m_target_B = b;

        m_fade_duration = duration_ms;
        m_last_update = millis();

        m_state = STATE::FADING;
        m_blink_mode = BLINK_MODE::NONE;
    }

    void RGB_LED::StartBlinking(uint8_t r, uint8_t g, uint8_t b,
                                uint16_t on_time_ms, uint16_t off_time_ms,
                                BLINK_MODE mode, uint16_t count)
    {
        m_target_R = r;
        m_target_G = g;
        m_target_B = b;

        m_blink_on_time = on_time_ms;
        m_blink_off_time = off_time_ms;
        m_blink_mode = mode;
        m_blink_count = 0;
        m_blink_max_count = count;

        m_blink_timer = millis();
        m_is_on = false;
        m_state = STATE::BLINKING;

        // Start with LED off
        m_current_R = 0;
        m_current_G = 0;
        m_current_B = 0;
        WriteToHardware();
    }

    void RGB_LED::StartBreathing(uint8_t r, uint8_t g, uint8_t b, uint16_t period_ms)
    {
        m_target_R = r;
        m_target_G = g;
        m_target_B = b;

        m_fade_duration = period_ms;
        m_breathing_phase = 0;
        m_last_update = millis();

        m_state = STATE::BREATHING;
        m_blink_mode = BLINK_MODE::NONE;
    }

    void RGB_LED::StopBlinking()
    {
        m_blink_mode = BLINK_MODE::NONE;
        m_state = STATE::SOLID;
    }

    void RGB_LED::Off()
    {
        SetColor(0, 0, 0);
    }

    RGB_LED::STATE RGB_LED::GetState() const
    {
        return m_state;
    }

    void RGB_LED::GetCurrentColor(uint8_t& r, uint8_t& g, uint8_t& b) const
    {
        r = m_current_R;
        g = m_current_G;
        b = m_current_B;
    }

    void RGB_LED::UpdateFading(unsigned long current_time)
    {
        unsigned long elapsed = current_time - m_last_update;

        if (elapsed >= m_fade_duration)
        {
            // Fade complete
            m_current_R = m_target_R;
            m_current_G = m_target_G;
            m_current_B = m_target_B;

            WriteToHardware();

            if (m_current_R == 0 && m_current_G == 0 && m_current_B == 0)
            {
                m_state = STATE::OFF;
            }
            else
            {
                m_state = STATE::SOLID;
            }

            return;
        }

        // Calculate interpolated values
        float progress = (float)elapsed / (float)m_fade_duration;

        m_current_R = m_current_R + (int16_t)((m_target_R - m_current_R) * progress);
        m_current_G = m_current_G + (int16_t)((m_target_G - m_current_G) * progress);
        m_current_B = m_current_B + (int16_t)((m_target_B - m_current_B) * progress);

        WriteToHardware();
    }

    void RGB_LED::UpdateBlinking(unsigned long current_time)
    {
        if (m_blink_mode == BLINK_MODE::NONE)
        {
            return;
        }

        unsigned long elapsed = current_time - m_blink_timer;
        uint16_t threshold = m_is_on ? m_blink_on_time : m_blink_off_time;

        if (elapsed >= threshold)
        {
            m_blink_timer = current_time;
            m_is_on = !m_is_on;

            if (m_is_on)
            {
                m_current_R = m_target_R;
                m_current_G = m_target_G;
                m_current_B = m_target_B;

                // Increment blink count when turning on
                if (m_blink_mode == BLINK_MODE::COUNT_LIMITED)
                {
                    m_blink_count++;

                    if (m_blink_count >= m_blink_max_count)
                    {
                        m_blink_mode = BLINK_MODE::NONE;
                        m_state = STATE::OFF;
                    }
                }
            }
            else
            {
                m_current_R = 0;
                m_current_G = 0;
                m_current_B = 0;
            }

            WriteToHardware();
        }
    }

    void RGB_LED::UpdateBreathing(unsigned long current_time)
    {
        unsigned long elapsed = current_time - m_last_update;

        if (elapsed >= m_fade_duration)
        {
            m_last_update = current_time;
            m_breathing_phase = 0;
        }

        // Calculate sine wave for smooth breathing effect
        float progress = (float)elapsed / (float)m_fade_duration;
        float sine_value = sin(progress * 2.0 * PI);

        // Map sine wave from [-1, 1] to [0, 1]
        float brightness = (sine_value + 1.0) / 2.0;

        m_current_R = (uint8_t)(m_target_R * brightness);
        m_current_G = (uint8_t)(m_target_G * brightness);
        m_current_B = (uint8_t)(m_target_B * brightness);

        WriteToHardware();
    }

    void RGB_LED::WriteToHardware()
    {
        Vfb_AnalogWrite(m_pin_R, m_current_R);
        Vfb_AnalogWrite(m_pin_G, m_current_G);
        Vfb_AnalogWrite(m_pin_B, m_current_B);
    }

} /* namespace Drivers */
