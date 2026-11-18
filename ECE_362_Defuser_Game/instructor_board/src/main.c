
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
 * Display the current strikes on the TFT.
 * Shows: STRIKES 0 OF 3, 1 OF 3, etc.
 */
static void draw_strikes(uint8_t strikes)
{
    // Clear the previous strike display
    tft_fill_rect(
        20, 260, 200, 35,
        TFT_PANEL
    );

    tft_draw_text(
        30, 270,
        "STRIKES",
        TFT_LIGHT_GRAY,
        2
    );

    tft_draw_char(
        126, 270,
        (char)('0' + strikes),
        strikes > 0 ? TFT_RED : TFT_GREEN,
        2
    );

    tft_draw_text(
        144, 270,
        "OF 3",
        TFT_LIGHT_GRAY,
        2
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

    draw_strikes(0);
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
 * Display maze puzzle instructions.
 */
static void draw_maze_screen(void)
{
    tft_fill_screen(TFT_BACKGROUND);

    tft_fill_rect(
        8, 12, 224, 296,
        TFT_PANEL
    );

    tft_draw_rect(
        8, 12, 224, 296,
        TFT_BORDER
    );

    tft_draw_text_centered(
        43, "DEFUSER GAME", TFT_CYAN, 2
    );

    tft_fill_rect(
        24, 79, 192, 2,
        TFT_BORDER
    );

    tft_draw_text_centered(
        101, "PUZZLE 2", TFT_WHITE, 3
    );

    tft_draw_text_centered(
        157, "MAZE PUZZLE", TFT_WHITE, 2
    );

    tft_draw_text_centered(
        206, "GUIDE THE SOLVER",
        TFT_LIGHT_GRAY, 2
    );

    tft_draw_text_centered(
        225, "USE ARROW KEYS",
        TFT_CYAN, 2
    );
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
    uint8_t strikes = 0;
    bool game_over = false;
    
    // Temporary directional sequence test
    const uint8_t test_sequence[4] = {0, 3, 1, 2};

    uint8_t sequence_index = 0;
    bool expecting_movement = false;
    while (true)
    {
        uint8_t message;

        while (uart_instructor_read(&message))
        {
            
            if (game_over)
            {
                // Ignore incoming messages after game over
                continue;
            }

            // Password completed: process maze button inputs
            if (keypad_complete)
            {
                // First byte identifies a movement packet
                if (!expecting_movement)
                {
                    if (message == UART_MAZE_MOVE)
                    {
                        expecting_movement = true;
                    }
                }
                else
                {
                    // Second byte specifies the direction
                    expecting_movement = false;

                    if (message <= 3)
                    {
                        if (message == test_sequence[sequence_index])
                        {
                            // Correct next direction
                            sequence_index++;

                            if (sequence_index == 4)
                            {
                                // All directions entered correctly
                                uart_instructor_send(UART_SUCCESS);

                                tft_fill_screen(TFT_BACKGROUND);

                                tft_draw_text_centered(
                                    120,
                                    "BOMB DEFUSED",
                                    TFT_GREEN,
                                    2
                                );

                                game_over = true;
                            }
                        }
                        else
                        {
                            // Incorrect direction
                            sequence_index = 0;
                            strikes++;

                            // Update strike counter on TFT
                            draw_strikes(strikes);

                            if (strikes >= 3)
                            {
                                // Three strikes: game over
                                game_over = true;

                                uart_instructor_send(UART_GAME_OVER);

                                tft_fill_screen(TFT_BACKGROUND);

                                tft_draw_text_centered(
                                    110, "GAME OVER", TFT_RED, 3
                                );

                                tft_draw_text_centered(
                                    175, "THREE STRIKES", TFT_WHITE, 2
                                );
                            }
                            else
                            {
                                // Allow another attempt
                                uart_instructor_send(UART_FAILURE);
                            }
                        }
                    }
                }

                // Don't process maze bytes as password bytes
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

            
            /*
            * Reject invalid password data.
            * Any value outside 0-9 counts as a strike.
            */
            if (message > 9)
            {
                // Reset password reception
                receiving_password = false;
                received_count = 0;

                // Add a strike
                strikes++;

                // Update the TFT strike counter
                draw_strikes(strikes);

                if (strikes >= 3)
                {
                    // Three strikes: game over
                    game_over = true;

                    uart_instructor_send(UART_GAME_OVER);

                    // Display game-over screen
                    tft_fill_screen(TFT_BACKGROUND);

                    tft_draw_text_centered(
                        110, "GAME OVER", TFT_RED, 3
                    );

                    tft_draw_text_centered(
                        175, "THREE STRIKES",
                        TFT_WHITE, 2
                    );
                }
                else
                {
                    // Reject answer and allow another attempt
                    uart_instructor_send(UART_FAILURE);
                }

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
                // Correct password
                keypad_complete = true;

                uart_instructor_send(UART_SUCCESS);

                // Advance to maze
                draw_maze_screen();

                // Preserve strikes from password puzzle
                draw_strikes(strikes);
            }
            else
            {
                // Incorrect password
                strikes++;

                // Update the TFT strike counter
                draw_strikes(strikes);
                if (strikes >= 3)
                {
                    // Three strikes: game over
                    game_over = true;

                    uart_instructor_send(UART_GAME_OVER);

                    // Display defeat on TFT
                    tft_fill_screen(TFT_BACKGROUND);

                    tft_draw_text_centered(
                        110, "GAME OVER", TFT_RED, 3
                    );

                    tft_draw_text_centered(
                        175, "THREE STRIKES", TFT_WHITE, 2
                    );
                }
                else
                {
                    // Allow another attempt
                    uart_instructor_send(UART_FAILURE);
                }
            }

            }
        }

        sleep_ms(10);
    }

    return 0;
}
