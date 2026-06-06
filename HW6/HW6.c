#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

// ================= I2C / MPU6050 =================
#define I2C_PORT i2c0
#define SDA_PIN 16
#define SCL_PIN 17

#define MPU_ADDR 0x68

#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B
#define ACCEL_CONFIG  0x1C
#define GYRO_CONFIG   0x1B

// ================= GLOBALS =================
int16_t ax_raw = 0, ay_raw = 0;

float ax_offset = 0;
float ay_offset = 0;

// ================= I2C HELPERS =================
void mpu_write(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
}

void mpu_read_accel(int16_t *ax, int16_t *ay)
{
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t data[6];

    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, data, 6, false);

    *ax = (int16_t)((data[0] << 8) | data[1]);
    *ay = (int16_t)((data[2] << 8) | data[3]);
}

// ================= MPU INIT =================
void mpu_init()
{
    mpu_write(PWR_MGMT_1, 0x00);   // wake up
    mpu_write(ACCEL_CONFIG, 0x00); // ±2g
    mpu_write(GYRO_CONFIG, 0x18);   // ±2000 dps
    sleep_ms(100);
}

// ================= CALIBRATION =================
void calibrate_mpu()
{
    printf("Calibrating... keep still\n");

    int samples = 200;
    int32_t sx = 0, sy = 0;

    for (int i = 0; i < samples; i++)
    {
        mpu_read_accel(&ax_raw, &ay_raw);
        sx += ax_raw;
        sy += ay_raw;
        sleep_ms(5);
    }

    ax_offset = sx / (float)samples;
    ay_offset = sy / (float)samples;

    printf("Calibration done: X=%f Y=%f\n", ax_offset, ay_offset);
}

// ================= HID MOUSE =================
static uint32_t blink_interval_ms = 250;

void led_blinking_task(void);
void hid_task(void);

static void send_mouse()
{
    if (!tud_hid_ready()) return;

    // read sensor
    mpu_read_accel(&ax_raw, &ay_raw);

    // remove offset
    float ax = ax_raw - ax_offset;
    float ay = ay_raw - ay_offset;

    // scale
    float dx = ax * 0.0008f;
    float dy = ay * 0.0008f;

    // deadzone (VERY IMPORTANT)
    if (fabsf(dx) < 0.8f) dx = 0;
    if (fabsf(dy) < 0.8f) dy = 0;

    // clamp
    if (dx > 5) dx = 5;
    if (dx < -5) dx = -5;
    if (dy > 5) dy = 5;
    if (dy < -5) dy = -5;

    tud_hid_mouse_report(
        REPORT_ID_MOUSE,
        0,
        (int8_t)dx,
        (int8_t)dy,
        0,
        0
    );
}

// ================= MAIN =================
int main()
{
    board_init();
    tusb_init();
    stdio_init_all();

    sleep_ms(2000);
    printf("Booting MPU mouse...\n");

    // I2C INIT
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(100);

    mpu_init();
    calibrate_mpu();

    while (1)
    {
        tud_task();
        hid_task();
        led_blinking_task();
    }
}

// ================= HID LOOP =================
void hid_task()
{
    static uint32_t last = 0;

    if (board_millis() - last < 10) return;
    last += 10;

    send_mouse();
}

// ================= LED =================
void led_blinking_task()
{
    static uint32_t last = 0;
    static bool led = false;

    if (board_millis() - last < blink_interval_ms) return;
    last += blink_interval_ms;

    board_led_write(led);
    led = !led;
}

// ================= REQUIRED CALLBACKS =================
void tud_mount_cb(void) { blink_interval_ms = 1000; }
void tud_umount_cb(void) { blink_interval_ms = 250; }
void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en; blink_interval_ms = 2500; }
void tud_resume_cb(void) { blink_interval_ms = 1000; }

uint16_t tud_hid_get_report_cb(uint8_t instance,
                              uint8_t report_id,
                              hid_report_type_t report_type,
                              uint8_t* buffer,
                              uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance,
                          uint8_t report_id,
                          hid_report_type_t report_type,
                          uint8_t const* buffer,
                          uint16_t bufsize)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}