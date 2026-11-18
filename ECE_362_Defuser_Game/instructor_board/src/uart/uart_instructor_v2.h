#ifndef UART_INSTRUCTOR_V2_H
#define UART_INSTRUCTOR_V2_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// UART0 configuration
#define INSTRUCTOR_UART_BAUD   115200
#define INSTRUCTOR_UART_TX_PIN 36
#define INSTRUCTOR_UART_RX_PIN 37

// Shared message definitions
// Must match solver-board UART v2
#define UART_FAILURE 0x55
#define UART_SUCCESS 0xAA
#define UART_READY   0x67
#define UART_ACK     0x28
#define UART_GO      0x96
#define UART_PASSWORD_SUBMIT 0xC1
#define UART_GAME_OVER 0xD0
#define UART_MAZE_MOVE 0xC2
// Initialize UART communication
void uart_instructor_init(void);

// Read one received byte
bool uart_instructor_read(uint8_t *message);

// Send one control byte
void uart_instructor_send(uint8_t message);

// Send an array of bytes
void uart_instructor_send_bytes(
    const uint8_t *data,
    size_t length
);

#endif
