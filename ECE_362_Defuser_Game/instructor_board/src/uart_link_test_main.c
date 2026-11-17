/*
 * ECE 362 - Defuser Game
 * Instructor-to-Solver UART Integration Test
 *
 * Test sequence:
 * 1. Send READY until ACK is received.
 * 2. Send GO to start the game.
 * 3. Send three SUCCESS messages.
 * 4. Stop transmitting test events.
 *
 * Expected solver LED behavior:
 * - Solid green at startup
 * - Green puzzle-complete animation
 * - Green puzzle-complete animation
 * - Green victory animation
 *
 * This is a test only, not the final game.
 */

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "uart/uart_instructor_v2.h"

#define HANDSHAKE_INTERVAL_MS 500
#define SUCCESS_INTERVAL_MS   3000

typedef enum {
    TEST_WAIT_ACK,
    TEST_RUNNING,
    TEST_COMPLETE
} test_state_t;

int main(void)
{
    stdio_init_all();
    uart_instructor_init();

    test_state_t state = TEST_WAIT_ACK;

    uint32_t last_ready = 0;
    uint32_t last_success = 0;
    uint8_t success_count = 0;

    // First handshake request
    uart_instructor_send(UART_READY);

    last_ready =
        to_ms_since_boot(get_absolute_time());

    while (true)
    {
        uint32_t now =
            to_ms_since_boot(get_absolute_time());

        uint8_t message;

        // Read incoming messages
        while (uart_instructor_read(&message))
        {
            if (state == TEST_WAIT_ACK &&
                message == UART_ACK)
            {
                // Solver acknowledged connection
                uart_instructor_send(UART_GO);

                state = TEST_RUNNING;
                last_success = now;
            }
        }

        switch (state)
        {
            case TEST_WAIT_ACK:
                // Retry handshake until connected
                if ((uint32_t)(now - last_ready) >=
                    HANDSHAKE_INTERVAL_MS)
                {
                    uart_instructor_send(UART_READY);
                    last_ready = now;
                }
                break;

            case TEST_RUNNING:
                // Simulate completed puzzles
                if ((uint32_t)(now - last_success) >=
                    SUCCESS_INTERVAL_MS)
                {
                    uart_instructor_send(UART_SUCCESS);
                    success_count++;
                    last_success = now;

                    if (success_count >= 3) {
                        state = TEST_COMPLETE;
                    }
                }
                break;

            case TEST_COMPLETE:
                // All test messages have been sent
                break;
        }

        sleep_ms(10);
    }

    return 0;
}
