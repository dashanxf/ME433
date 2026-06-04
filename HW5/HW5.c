#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ssd1306.h"

// ================= I2C =================
#define I2C_PORT i2c0
#define SDA_PIN 16
#define SCL_PIN 17

#define MPU6050_ADDR 0x68

// ================= MPU6050 REGISTERS =================
#define ACCEL_XOUT_H  0x3B
#define WHO_AM_I      0x75
#define PWR_MGMT_1    0x6B
#define ACCEL_CONFIG  0x1C
#define GYRO_CONFIG   0x1B

// ================= I2C HELPERS =================
void write_reg(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

uint8_t read_reg(uint8_t reg) {
    uint8_t val;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, &val, 1, false);
    return val;
}

void burst_read(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buf, len, false);
}

int16_t combine(uint8_t h, uint8_t l) {
    return (int16_t)((h << 8) | l);
}

// ================= MPU INIT =================
void mpu_init() {
    write_reg(PWR_MGMT_1, 0x00);   // wake up
    write_reg(ACCEL_CONFIG, 0x00);  // ±2g
    write_reg(GYRO_CONFIG, 0x18);   // ±2000 dps
    sleep_ms(100);
}

// ================= LINE DRAW =================
void drawLine(int x0, int y0, int x1, int y1) {

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;

    int err = dx + dy;

    while (1) {

        ssd1306_drawPixel(x0, y0, 1);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// ================= MAIN =================
int main() {

    stdio_init_all();
    sleep_ms(2000);

    // -------- I2C INIT --------
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    sleep_ms(100);

    // -------- OLED INIT --------
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    // -------- MPU CHECK --------
    uint8_t who = read_reg(WHO_AM_I);

    if (who != 0x68 && who != 0x98) {
        while (1) {
            tight_loop_contents(); // sensor not found
        }
    }

    mpu_init();

    uint8_t data[14];

    // ================= LOOP =================
    while (1) {

        burst_read(ACCEL_XOUT_H, data, 14);

        int16_t ax_raw = combine(data[0], data[1]);
        int16_t ay_raw = combine(data[2], data[3]);

        float ax = ax_raw * 0.000061f;
        float ay = ay_raw * 0.000061f;

        // -------- OLED DRAW --------
        ssd1306_clear();

        int cx = 64;
        int cy = 16;

        int x = cx + (int)(ax * 30.0f);
        int y = cy - (int)(ay * 15.0f);

        // center marker
        ssd1306_drawPixel(cx, cy, 1);
        ssd1306_drawPixel(cx+1, cy, 1);
        ssd1306_drawPixel(cx-1, cy, 1);
        ssd1306_drawPixel(cx, cy+1, 1);
        ssd1306_drawPixel(cx, cy-1, 1);

        // gravity vector line
        drawLine(cx, cy, x, y);

        ssd1306_update();

        sleep_ms(10); // ~100 Hz
    }
}
