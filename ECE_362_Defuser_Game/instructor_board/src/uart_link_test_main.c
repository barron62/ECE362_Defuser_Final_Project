
/*
 * ECE 362 - Defuser Game
 * Instructor Board Password Validation Test
 *
 * UART1:
 * TX = GP36
 * RX = GP37
 * Baud rate = 115200
 *
 * Protocol:
 * READY -> ACK -> GO
 *
 * Password packet:
 * 0xC1, digit0, digit1, digit2, digit3
 *
 * Test password: 1234
 *
 * Correct: UART_SUCCESS (0xAA)
 * Incorrect: UART_FAILURE (0x55)
 *
 * No automatic puzzle completion.
 */

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "uart/uart_instructor_v2.h"

#define PASSWORD_LENGTH 4
#define READY_INTERVAL_MS 500

static const uint8_t correct_password[PASSWORD_LENGTH] =
{
    1, 2, 3, 4
};

/*
 * Wait for solver handshake before starting game.
 */
static void wait_for_solver(void)
{
    bool acknowledged = false;

    while (!acknowledged)
    {
        uart_instructor_send(UART_READY);

        for (int i = 0; i < 50; i++)
        {
            uint8_t message;

            while (uart_instructor_read(&message))
            {
                if (message == UART_ACK)
                {
                    acknowledged = true;
                }
            }

            if (acknowledged)
            {
                break;
            }

            sleep_ms(READY_INTERVAL_MS / 50);
        }
    }
}

/*
 * Check whether four received digits are correct.
 */
static bool password_matches(const uint8_t *answer)
{
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        if (answer[i] != correct_password[i])
        {
            return false;
        }
    }

    return true;
}

/*
 * Instructor game test.
 */
int main(void)
{
    uart_instructor_init();

    // Establish connection
    wait_for_solver();

    // Start keypad puzzle
    uart_instructor_send(UART_GO);

    uint8_t received_password[PASSWORD_LENGTH];
    uint8_t received_count = 0;

    bool receiving_password = false;
    bool keypad_complete = false;

    while (true)
    {
        uint8_t message;

        while (uart_instructor_read(&message))
        {
            // Ignore additional passwords after
            // the keypad puzzle is complete.
            if (keypad_complete)
            {
                continue;
            }

            // Wait for beginning of password packet
            if (!receiving_password)
            {
                if (message == UART_PASSWORD_SUBMIT)
                {
                    receiving_password = true;
                    received_count = 0;
                }

                continue;
            }

            // Receive numeric password digits
            if (message > 9)
            {
                // Invalid payload: reject
                receiving_password = false;
                received_count = 0;
                uart_instructor_send(UART_FAILURE);
                continue;
            }

            received_password[received_count] = message;
            received_count++;

            // Validate after receiving all four digits
            if (received_count == PASSWORD_LENGTH)
            {
                receiving_password = false;
                received_count = 0;

                if (password_matches(received_password))
                {
                    keypad_complete = true;
                    uart_instructor_send(UART_SUCCESS);
                }
                else
                {
                    uart_instructor_send(UART_FAILURE);
                }
            }
        }

        sleep_ms(10);
    }

    return 0;
}
