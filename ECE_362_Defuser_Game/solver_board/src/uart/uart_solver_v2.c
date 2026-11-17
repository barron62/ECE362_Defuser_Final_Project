
#include "uart_solver_v2.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"

// RP2350 UART0
#define SOLVER_UART uart0

void uart_solver_init(void)
{
    uart_init(SOLVER_UART, SOLVER_UART_BAUD);

    gpio_set_function(SOLVER_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(SOLVER_UART_RX_PIN, GPIO_FUNC_UART);
}

bool uart_solver_read(uint8_t *message)
{
    if (message == NULL || !uart_is_readable(SOLVER_UART)) {
        return false;
    }

    *message = (uint8_t)uart_getc(SOLVER_UART);
    return true;
}

void uart_solver_send(uint8_t message)
{
    uart_putc_raw(SOLVER_UART, message);
}
