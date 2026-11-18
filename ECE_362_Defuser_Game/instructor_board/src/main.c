/*
 * ECE 362 - Defuser Game
 * Instructor Board - Main Game Controller
 *
 * Hardware:
 * - RP2350 Proton
 * - UART1: GP36 TX, GP37 RX
 * - SPI0 TFT display
 *
 * Game behavior:
 * - Generate and validate random password
 * - Communicate with solver through UART
 * - Track three strikes across both puzzles
 * - Run shared three-minute countdown
 * - Process directional maze inputs
 * - Handle victory and game-over conditions
 *
 * Current maze uses a temporary direction sequence.
 */

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"

#include "uart/uart_instructor_v2.h"
#include "tft/tft_display.h"
#include "timer/game_timer.h"
#include "game_screens/game_screens.h"
#include "password/password.h"
#include "maze/maze.h"
#include "game/game_controller.h"
#define READY_INTERVAL_MS 500


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
 * Main instructor program.
 */
int main(void)
{
    // Initialize hardware
    uart_instructor_init();
    tft_init();
    game_timer_init();
    // Generate random password
    password_init();
    maze_init();
    game_init();
    // Display password on instructor TFT
    game_screen_password(password_get_code());

    // Wait for solver to connect
    wait_for_solver();

    // Start keypad puzzle
    uart_instructor_send(UART_GO);
    // Begin the three-minute countdown
    game_timer_start();
    uint32_t last_displayed_time = UINT32_MAX;
    // UART password receive buffer
    uint8_t received_password[PASSWORD_LENGTH];
    uint8_t received_count = 0;

    bool receiving_password = false;
    bool keypad_complete = false;
    bool expecting_movement = false;
    
    while (true)
    {
        uint32_t seconds_left = game_timer_remaining_seconds();

        // Check for timeout first
        if (!game_is_over() && game_timer_expired())
        {
            game_time_expired();
        }

        // Update TFT timer once per second
        if (!game_is_over() &&
            seconds_left != last_displayed_time)
        {
            tft_draw_timer(seconds_left);
            last_displayed_time = seconds_left;
        }


        uint8_t message;

        while (uart_instructor_read(&message))
        {
            
            if (game_is_over())
            {
                // Ignore incoming messages after game over
                continue;
            }

            // Password completed: process maze button inputs
            
            if (keypad_complete)
            {
                if (!expecting_movement)
                {
                    if (message == UART_MAZE_MOVE)
                    {
                        expecting_movement = true;
                    }
                }
                else
                {
                    expecting_movement = false;

                    maze_result_t result =
                        maze_process_direction(message);
                    
                    if (result == MAZE_COMPLETE)
                    {
                        // Maze completed successfully
                        game_win();
                    }
                    else if (result == MAZE_INCORRECT)
                    {
                        // Incorrect movement adds a strike
                        game_register_strike();
                    }

                }

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

                game_register_strike();

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
                    game_screen_maze();

                    // Preserve strikes from password puzzle
                    game_screen_strikes(game_get_strikes());

                    // Force timer to redraw on new screen
                    last_displayed_time = UINT32_MAX;
                }
                else
                {
                    game_register_strike();
                }

            }
        }

        sleep_ms(10);
    }

    return 0;
}
