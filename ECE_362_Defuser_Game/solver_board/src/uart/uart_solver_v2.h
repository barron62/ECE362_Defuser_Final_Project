
#ifndef UART_SOLVER_V2_H
#define UART_SOLVER_V2_H

#include <stdint.h>
#include <stdbool.h>

// UART hardware configuration
#define SOLVER_UART_BAUD 115200
#define SOLVER_UART_TX_PIN 0
#define SOLVER_UART_RX_PIN 1

// Shared protocol values
#define UART_FAILURE 0x55
#define UART_SUCCESS 0xAA
#define UART_READY   0x67
#define UART_ACK     0x28
#define UART_GO      0x96

// Initialize solver UART
void uart_solver_init(void);

// Read one received byte, if available
bool uart_solver_read(uint8_t *message);

// Transmit a single byte
void uart_solver_send(uint8_t message);

#endif
