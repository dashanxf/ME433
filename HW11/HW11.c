#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 12   // Pico sends to STM32 RX
#define UART_RX_PIN 13   // Pico receives from STM32 TX

int main()
{
    // USB serial (printf)
    stdio_init_all();

    // UART setup
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    printf("Pico UART bridge started\r\n");
    char buffer[128];
    while (true)
    {
        // =========================
        // STM32 -> Pico -> USB
        // =========================
        if (uart_is_readable(UART_ID))
        {
            char c = uart_getc(UART_ID);
            printf("%c", c);
        }

        // =========================
        // Send Pico -> STM32 (test string)
        // =========================
        int ch = getchar_timeout_us(0);

        if (ch != PICO_ERROR_TIMEOUT)
        {
            char c = (char)ch;

            char msg[3];
            msg[0] = c;
            msg[1] = '\r';
            msg[2] = '\n';

            printf("Sending: %c\n", msg[0]);

            uart_puts(UART_ID, msg);
            sleep_ms(1);
        }
        //uart_puts(UART_ID, "A\r\n");
        //sleep_ms(1);
    }
}
