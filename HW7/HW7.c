#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

#define PIN_CS 17
#define SPI_PORT spi0

#define DAC_MAX 4095
#define VREF 3.3

#define SINE_FREQ 2
#define TRI_FREQ 1

#define UPDATE_RATE 1000   // 1 kHz
#define SINE_SAMPLES (UPDATE_RATE / SINE_FREQ)
#define TRI_SAMPLES (UPDATE_RATE / TRI_FREQ)

uint16_t sine_wave[SINE_SAMPLES];
uint16_t triangle_wave[TRI_SAMPLES];

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); 
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); 
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); 
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); 
}

void dac_write(uint16_t value) {
    uint8_t data[2];
    uint16_t command = 0x3000 | (value & 0x0FFF); // DAC A, unbuffered, gain 1x

    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

void generate_waveforms() {
    for(int i=0; i<SINE_SAMPLES; i++) {
        double t = (double)i / UPDATE_RATE;
        double v = (sin(2.0 * M_PI * SINE_FREQ * t) + 1.0) / 2.0; // 0–1
        sine_wave[i] = (uint16_t)(v * DAC_MAX);
    }

    for(int i=0; i<TRI_SAMPLES; i++) {
        double t = (double)i / UPDATE_RATE;
        double v = fmod(TRI_FREQ * t, 1.0); // ramp 0–1
        triangle_wave[i] = (uint16_t)(v * DAC_MAX);
    }
}

int main() {
    stdio_init_all();
    spi_init(SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, true);
    cs_deselect(PIN_CS);

    generate_waveforms();

    while(1) {
        // Output sine wave
        // for(int i=0; i<SINE_SAMPLES; i++) {
        //     dac_write(sine_wave[i]);
        //     sleep_us(1000000 / UPDATE_RATE); // delay between samples
        // }

        // Output triangle wave
        for(int i=0; i<TRI_SAMPLES; i++) {
            dac_write(triangle_wave[i]);
            sleep_us(1000000 / UPDATE_RATE);
        }
    }
}
