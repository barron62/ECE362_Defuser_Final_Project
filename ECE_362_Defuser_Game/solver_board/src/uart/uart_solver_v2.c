
#include "uart_solver_v2.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_PORT uart1

void uart_solver_init(void)
{
    uart_init(UART_PORT, SOLVER_UART_BAUD);

    gpio_set_function(SOLVER_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(SOLVER_UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE);
    uart_set_hw_flow(UART_PORT, false, false);
}

bool uart_solver_read(uint8_t *message)
{
    if (message == NULL || !uart_is_readable(UART_PORT))
    {
        return false;
    }

    *message = (uint8_t)uart_getc(UART_PORT);
    return true;
}

void uart_solver_send(uint8_t message)
{
    uart_putc_raw(UART_PORT, message);
}
