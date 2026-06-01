#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17

#define MCP23008_ADDR 0x20

// MCP23008 Registers
#define IODIR 0x00
#define GPIO  0x09
#define OLAT  0x0A

void writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;

    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

uint8_t readRegister(uint8_t address, uint8_t reg) {
    uint8_t value;

    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);

    return value;
}

int main() {

    stdio_init_all();

    // Initialize I2C
    i2c_init(I2C_PORT, 100000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    //gpio_pull_up(I2C_SDA);
    //gpio_pull_up(I2C_SCL);

    sleep_ms(100);

    // GP7 = output (0)
    // GP0-GP6 = input (1)
    // Binary: 01111111
    writeRegister(MCP23008_ADDR, IODIR, 0x7F);

    // Start with LED off
    writeRegister(MCP23008_ADDR, OLAT, 0x00);

    while (true) {

        uint8_t gpio_state = readRegister(MCP23008_ADDR, GPIO);

        // Check GP0
        if ((gpio_state & (1 << 0))) {

            // Turn ON GP7 LED
            uint8_t olat = readRegister(MCP23008_ADDR, OLAT);
            olat |= (1 << 7);
            writeRegister(MCP23008_ADDR, OLAT, olat);

        } else {

            // Turn OFF GP7 LED
            uint8_t olat = readRegister(MCP23008_ADDR, OLAT);
            olat &= ~(1 << 7);
            writeRegister(MCP23008_ADDR, OLAT, olat);
        }

        sleep_ms(10);
    }
}
