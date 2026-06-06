#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 15

// Convert pulse width (in microseconds) to PWM level
uint16_t us_to_level(uint slice_num, float pulse_us) {
    // PWM clock is 125 MHz by default
    // We'll scale using wrap value
    uint32_t clock = 125000000;
    uint32_t divider = 64;  // chosen divider
    uint32_t wrap = 39062;  // gives ~50 Hz

    float counts_per_us = (clock / divider) / 1e6;
    return (uint16_t)(pulse_us * counts_per_us);
}

void set_servo_angle(uint slice_num, uint channel, float angle) {
    // Map angle (0–180) to pulse width (1000–2000 us)
    float pulse = 1000 + (angle / 180.0f) * 1000;
    uint16_t level = us_to_level(slice_num, pulse);

    pwm_set_chan_level(slice_num, channel, level);
}

int main() {
    stdio_init_all();

    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    uint channel = pwm_gpio_to_channel(SERVO_PIN);

    // Set PWM config
    pwm_set_clkdiv(slice_num, 64.0f);     // clock divider
    pwm_set_wrap(slice_num, 39062);       // for ~50 Hz

    pwm_set_enabled(slice_num, true);

    while (true) {
        // Sweep servo
        for (float angle = 0; angle <= 180; angle += 5) {
            set_servo_angle(slice_num, channel, angle);
            sleep_ms(50);
        }

        for (float angle = 180; angle >= 0; angle -= 5) {
            set_servo_angle(slice_num, channel, angle);
            sleep_ms(50);
        }
    }
}