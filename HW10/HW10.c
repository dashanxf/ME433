#include <stdio.h>
#include "pico/stdlib.h"

#define BUTTON_PIN 9

volatile int press_count = 0;
bool last_state = true; // pull-up => not pressed = 1

int main() {
    stdio_init_all();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    while (true) {
        bool current_state = gpio_get(BUTTON_PIN);

        // Detect falling edge (button press)
        if (last_state == true && current_state == false) {
            press_count++;
            printf("Button pressed: %d times\n", press_count);
            sleep_ms(200); // simple debounce
        }

        last_state = current_state;
        sleep_ms(10);
    }
}