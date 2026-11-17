/*
 * ECE 362 - Defuser Game
 * Solver Board Main Program
 *
 * Hardware:
 * - RP2350 Proton microcontroller
 * - PIO-controlled NeoPixel LED bar
 * - Keypad and puzzle buttons
 * - UART communication with instructor board
 *
 * LED Behavior:
 * - Normal gameplay: Solid green
 * - 30 seconds remaining: Slow red flashing
 * - 15 seconds remaining: Faster red flashing
 * - 5 seconds remaining: Rapid red flashing
 * - Puzzle completed: Green animation
 * - Game won: Victory animation
 * - Game over: Solid red
 *
 * TODO:
 * - Integrate keypad and puzzle button inputs
 * - Implement complete puzzle answer transmission
 * - Receive remaining time from instructor board
 * - Implement explicit game-over message
 * - Verify UART protocol with instructor board
 */

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "led/led_strip.h"
#include "uart/uart_solver_v2.h"
#include "game/game_controller.h"
#include <stdio.h>
#include "keypad/keypad.h"
// Temporary GPIO assignment
// Update after confirming final PCB wiring
#define LED_PIN 16

/*
 * Initialize solver board hardware.
 */
static void init_hardware(void)
{
    stdio_init_all();

    // Initialize PIO NeoPixel controller
    if (!led_init(LED_PIN)) {
        while (true) {
            tight_loop_contents();
        }
    }

    // Initialize UART communication
    uart_solver_init();

    // Initialize game state machine
    game_init();

    // Default LED state
    led_running();

    // Initialize 4x4 keypad
    keypad_init();
}

/*
 * Process game events and update LED effects.
 */
static void handle_led_events(void)
{
    game_event_t event = game_get_event();

    switch (event)
    {
        case GAME_EVENT_STARTED:
            // New game started
            led_running();
            break;

        case GAME_EVENT_PUZZLE_COMPLETE:
            // Individual puzzle completed
            led_puzzle_complete();
            break;

        case GAME_EVENT_WON:
            // All three puzzles completed
            led_win();
            break;

        case GAME_EVENT_LOST:
            // Game over
            led_game_over();
            break;

        case GAME_EVENT_ANSWER_REJECTED:
            // Incorrect answer; allow retry
            break;

        case GAME_EVENT_NONE:
        default:
            break;
    }
}

/*
 * Main game program.
 */
int main(void)
{
    init_hardware();

    while (true)
    {
        // Receive UART messages from instructor board
        uint8_t message;

        while (uart_solver_read(&message)) {
            game_process_message(message);
        }

        // Read keypad inputs
        char key = keypad_get_key();

        if (key != '\0') {
            printf("Key pressed: %c\n", key);

            // TODO: Add key to password buffer
        }

        // TODO: Read puzzle button inputs

        // TODO: Send completed puzzle answers
        // using game_send_answer()

        // Process game events and trigger LEDs
        handle_led_events();

        // TODO: Update countdown from instructor board
        // led_set_time_remaining(time_remaining_ms);

        // Continuously update PIO animations
        led_update();

        sleep_ms(10);
    }

    return 0;
}
