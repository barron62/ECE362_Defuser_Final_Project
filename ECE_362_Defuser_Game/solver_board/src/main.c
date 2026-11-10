/*
 * ECE 362 - Defuser Game
 * Solver Board Main Program
 *
 * Hardware:
 * - RP2350 microcontroller
 * - PIO addressable LED bar
 * - Keypad and puzzle buttons
 * - UART communication with instructor board
 *
 * TODO:
 * - Integrate keypad and button controls
 * - Integrate UART communication
 * - Connect LED animations to game events
 */

#include "pico/stdlib.h"
#include "led/led_strip.h"

// Temporary GPIO assignment
// Update after confirming PCB wiring
#define LED_PIN 16

int main(void) {
    stdio_init_all();

    // Initialize PIO LED controller
    if (!led_init(LED_PIN)) {
        while (true) {
            tight_loop_contents();
        }
    }

    // Default LED state
    led_running();

    while (true) {

        // TODO: Read keypad inputs

        // TODO: Read puzzle buttons

        // TODO: Process UART messages

        // TODO: Trigger LED effects based on game state
        // led_puzzle_complete();
        // led_warning(time_remaining_ms);
        // led_win();
        // led_game_over();

        // Continuously update PIO LED animations
        led_update();

        sleep_ms(10);
    }

    return 0;
}
