
#ifndef UART_SOLVER_V2_H
#define UART_SOLVER_V2_H

#include <stdint.h>
#include <stdbool.h>

// UART hardware configuration
#define SOLVER_UART_BAUD 115200
#define SOLVER_UART_TX_PIN 36
#define SOLVER_UART_RX_PIN 37

// Shared protocol values
#define UART_FAILURE 0x55
#define UART_SUCCESS 0xAA
#define UART_READY   0x67
#define UART_ACK     0x28
#define UART_GO      0x96
#define UART_PASSWORD_SUBMIT 0xC1
#define UART_GAME_OVER 0xD0
#define UART_MAZE_MOVE 0xC2
// Initialize solver UART
void uart_solver_init(void);

// Read one received byte, if available
bool uart_solver_read(uint8_t *message);

// Transmit a single byte
void uart_solver_send(uint8_t message);

#endif
