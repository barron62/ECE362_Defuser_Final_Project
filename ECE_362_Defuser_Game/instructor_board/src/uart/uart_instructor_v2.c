/*
 * ECE 362 - Defuser Game
 * Instructor Board UART Driver V2
 *
 * Handles UART communication with solver board.
 *
 * Hardware:
 * - RP2350 Proton
 * - UART0
 * - GPIO0: TX
 * - GPIO1: RX
 * - Baud rate: 115200
 *
 * Uses polling for simple, predictable testing.
 */

#include "uart_instructor_v2.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"

#define INSTRUCTOR_UART uart0

/*
 * Initialize UART0 and assign GPIO pins.
 */
void uart_instructor_init(void)
{
    uart_init(INSTRUCTOR_UART, INSTRUCTOR_UART_BAUD);

    gpio_set_function(
        INSTRUCTOR_UART_TX_PIN,
        GPIO_FUNC_UART
    );

    gpio_set_function(
        INSTRUCTOR_UART_RX_PIN,
        GPIO_FUNC_UART
    );
}

/*
 * Read one byte if data is available.
 */
bool uart_instructor_read(uint8_t *message)
{
    if (message == NULL ||
        !uart_is_readable(INSTRUCTOR_UART)) {
        return false;
    }

    *message = (uint8_t)uart_getc(INSTRUCTOR_UART);

    return true;
}

/*
 * Transmit a single UART control byte.
 */
void uart_instructor_send(uint8_t message)
{
    uart_putc_raw(INSTRUCTOR_UART, message);
}

/*
 * Transmit multiple bytes.
 *
 * Used later for structured messages such as
 * time updates or other game information.
 */
void uart_instructor_send_bytes(
    const uint8_t *data,
    size_t length
)
{
    if (data == NULL || length == 0) {
        return;
    }

    uart_write_blocking(
        INSTRUCTOR_UART,
        data,
        length
    );
}
