#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"
#include <stdio.h>

#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17

#define LED_PIN 25

void drawChar(int x, int y, char c);
void drawMessage(int x, int y, char *msg);

int main() {

    stdio_init_all();

    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // I2C
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    sleep_ms(100);

    ssd1306_setup();

    // ADC init (ADC0 = GPIO26)
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    char msg[50];
    char fpsmsg[50];

    uint32_t last_time = to_us_since_boot(get_absolute_time());
    int frames = 0;

    while (true) {

        gpio_put(LED_PIN, 1);

        // read ADC
        uint16_t raw = adc_read();
        float voltage = raw * 3.3f / 4095.0f;

        // compute FPS
        frames++;
        uint32_t now = to_us_since_boot(get_absolute_time());

        float fps = 1000000.0f / (now - last_time);
        last_time = now;

        // build strings
        sprintf(msg, "V=%.2fV", voltage);
        sprintf(fpsmsg, "FPS=%.1f", fps);

        // draw
        ssd1306_clear();

        drawMessage(0, 0, msg);
        drawMessage(0, 16, fpsmsg);

        ssd1306_update();

        gpio_put(LED_PIN, 0);

        sleep_ms(100);
    }
}
