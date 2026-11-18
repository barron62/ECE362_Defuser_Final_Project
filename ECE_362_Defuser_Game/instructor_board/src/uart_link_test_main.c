
/*
 * ECE 362 - Defuser Game
 * Instructor Board - Random Password Puzzle
 *
 * Hardware:
 * - RP2350 Proton
 * - UART1: GP36 TX, GP37 RX
 * - SPI0 TFT display
 *
 * Game behavior:
 * 1. Generate a random four-digit password
 * 2. Display it on the instructor TFT
 * 3. Establish UART handshake with solver
 * 4. Start the keypad puzzle
 * 5. Receive password over UART
 * 6. Compare against generated password
 * 7. Send SUCCESS or FAILURE
 *
 * Correct password changes every startup.
 */

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/rand.h"

#include "uart/uart_instructor_v2.h"
#include "tft/tft_display.h"

#define PASSWORD_LENGTH 4
#define READY_INTERVAL_MS 500

// The generated password is stored here
static uint8_t correct_password[PASSWORD_LENGTH];

/*
 * Generate a new four-digit password.
 * Digits may include zero.
 */
static void generate_password(void)
{
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        correct_password[i] =
            (uint8_t)(get_rand_32() % 10);
    }
}

/*
 * Draw one password box and its digit.
 */
static void draw_password_box(
    uint16_t x,
    uint8_t digit)
{
    const uint16_t y = 199;
    const uint16_t width = 38;
    const uint16_t height = 50;

    tft_fill_rect(
        x, y, width, height,
        TFT_BACKGROUND
    );

    tft_draw_rect(
        x, y, width, height,
        TFT_BORDER
    );

    // Draw the actual password digit
    tft_draw_char(
        x + 12,
        y + 15,
        (char)('0' + digit),
        TFT_WHITE,
        3
    );
}

/*
 * Display the generated password
 * on the instructor TFT.
 */
static void draw_password_screen(void)
{
    tft_fill_screen(TFT_BACKGROUND);

    // Main panel
    tft_fill_rect(
        8, 12, 224, 296,
        TFT_PANEL
    );

    tft_draw_rect(
        8, 12, 224, 296,
        TFT_BORDER
    );

    // Game title
    tft_draw_text_centered(
        43,
        "DEFUSER GAME",
        TFT_CYAN,
        2
    );

    // Divider
    tft_fill_rect(
        24, 79, 192, 2,
        TFT_BORDER
    );

    // Puzzle title
    tft_draw_text_centered(
        101,
        "PUZZLE 1",
        TFT_WHITE,
        3
    );

    // Instructions
    tft_draw_text_centered(
        157,
        "PASSWORD CODE",
        TFT_WHITE,
        2
    );

    // Display the four generated digits
    draw_password_box(26, correct_password[0]);
    draw_password_box(76, correct_password[1]);
    draw_password_box(126, correct_password[2]);
    draw_password_box(176, correct_password[3]);

    // Instructor instruction
    tft_draw_text_centered(
        279,
        "TELL THE SOLVER",
        TFT_LIGHT_GRAY,
        2
    );
}

/*
 * Wait for the solver to acknowledge READY.
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
 * Compare a submitted password against
 * the generated password.
 */
static bool password_matches(
    const uint8_t *answer)
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
 * Main instructor program.
 */
int main(void)
{
    // Initialize hardware
    uart_instructor_init();
    tft_init();

    // Generate password once per game
    generate_password();

    // Display password on TFT
    draw_password_screen();

    // Wait for solver to connect
    wait_for_solver();

    // Start keypad puzzle
    uart_instructor_send(UART_GO);

    // UART password receive buffer
    uint8_t received_password[PASSWORD_LENGTH];
    uint8_t received_count = 0;

    bool receiving_password = false;
    bool keypad_complete = false;

    while (true)
    {
        uint8_t message;

        while (uart_instructor_read(&message))
        {
            // Ignore submissions after success
            if (keypad_complete)
            {
                continue;
            }

            // Wait for packet start
            if (!receiving_password)
            {
                if (message == UART_PASSWORD_SUBMIT)
                {
                    receiving_password = true;
                    received_count = 0;
                }

                continue;
            }

            // Only accept digits from 0 to 9
            if (message > 9)
            {
                receiving_password = false;
                received_count = 0;

                uart_instructor_send(UART_FAILURE);
                continue;
            }

            // Store incoming digit
            received_password[received_count] = message;
            received_count++;

            // Validate once all four digits arrive
            if (received_count == PASSWORD_LENGTH)
            {
                receiving_password = false;
                received_count = 0;

                if (password_matches(received_password))
                {
                    keypad_complete = true;

                    // Tell solver password was correct
                    uart_instructor_send(UART_SUCCESS);
                }
                else
                {
                    // Incorrect; allow another try
                    uart_instructor_send(UART_FAILURE);
                }
            }
        }

        sleep_ms(10);
    }

    return 0;
}
