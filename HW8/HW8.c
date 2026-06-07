#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0

// SPI pins
#define PIN_MISO 16
#define PIN_CS_DAC 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_CS_RAM 20

#define NUM_SAMPLES 1000

// SRAM commands
#define SRAM_READ  0x03
#define SRAM_WRITE 0x02
#define SRAM_WRSR  0x01

uint16_t sine_samples[NUM_SAMPLES];

// ======= CS helpers =======
static inline void cs_select(uint pin) { gpio_put(pin, 0); }
static inline void cs_deselect(uint pin) { gpio_put(pin, 1); }

// ======= DAC (MCP4912) =======
void dac_write(uint16_t value) {
    // MCP4912 command bits: 0b0011xxxx xxxx xxxx
    uint16_t command = 0x3000 | (value & 0x0FFF);

    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

// ======= SRAM (23K256) =======
void spi_ram_init() {
    // CS pin
    gpio_init(PIN_CS_RAM);
    gpio_set_dir(PIN_CS_RAM, true);
    cs_deselect(PIN_CS_RAM);

    // Set sequential mode
    uint8_t data[2] = {SRAM_WRSR, 0x40};
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_RAM);
}

void sram_write16(uint16_t address, uint16_t value) {
    uint8_t header[3] = {SRAM_WRITE, (address >> 8) & 0xFF, address & 0xFF};
    uint8_t payload[2] = {(value >> 8) & 0xFF, value & 0xFF};

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, header, 3);
    spi_write_blocking(SPI_PORT, payload, 2);
    cs_deselect(PIN_CS_RAM);
}

uint16_t sram_read16(uint16_t address) {
    uint8_t header[3] = {SRAM_READ, (address >> 8) & 0xFF, address & 0xFF};
    uint8_t result[2];

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, header, 3);
    spi_read_blocking(SPI_PORT, 0x00, result, 2);
    cs_deselect(PIN_CS_RAM);

    return ((uint16_t)result[0] << 8) | result[1];
}

// ======= Main =======
int main() {
    stdio_init_all();

    // Initialize SPI
    spi_init(SPI_PORT, 1 * 1000 * 1000); // 1 MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // DAC CS
    gpio_init(PIN_CS_DAC);
    gpio_set_dir(PIN_CS_DAC, true);
    cs_deselect(PIN_CS_DAC);

    // Initialize SRAM
    spi_ram_init();

    sleep_ms(100); // wait a moment

    // Generate sine samples (0–3.3V)
    for (int i = 0; i < NUM_SAMPLES; i++) {
        float theta = 2.0f * M_PI * ((float)i / NUM_SAMPLES);
        float voltage = 1.65f * (sinf(theta) + 1.0f); // 0–3.3V
        sine_samples[i] = (uint16_t)((voltage / 3.3f) * 4095.0f);
    }

    // Write samples to SRAM
    for (int i = 0; i < NUM_SAMPLES; i++) {
        sram_write16(i * 2, sine_samples[i]); // each sample = 2 bytes
    }

    printf("SRAM initialized and sine wave stored.\n");

    // Output sine wave from SRAM through DAC
    while (1) {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            uint16_t sample = sram_read16(i * 2);
            dac_write(sample);
            sleep_ms(1); // 1 kHz update rate -> 1Hz sine wave
        }
    }
}
