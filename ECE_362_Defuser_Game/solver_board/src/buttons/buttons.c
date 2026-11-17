/*
 * ECE 362 - Defuser Game
 * Solver Board Directional Button Driver
 *
 * Hardware:
 * - RP2350 Proton
 * - Four directional pushbuttons
 *
 * GPIO Assignments:
 * - GP10: Up
 * - GP11: Down
 * - GP12: Left
 * - GP13: Right
 *
 * Each button connects between GPIO and GND.
 * Internal pull-up resistors are enabled.
 *
 * Features:
 * - GPIO input polling
 * - Software debouncing (25 ms)
 * - One event per button press
 * - Non-blocking operation
 */

#include "buttons.h"
#include "pico/stdlib.h"

#include <stdint.h>
#include <stdbool.h>

#define BUTTON_COUNT 4

static const uint button_pins[BUTTON_COUNT] = {
    BUTTON_UP_PIN,
    BUTTON_DOWN_PIN,
    BUTTON_LEFT_PIN,
    BUTTON_RIGHT_PIN
};

static const direction_t button_directions[BUTTON_COUNT] = {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

static bool last_raw[BUTTON_COUNT];
static bool stable_pressed[BUTTON_COUNT];
static uint32_t last_change_ms[BUTTON_COUNT];

/*
 * Initialize button GPIO inputs.
 */
void buttons_init(void)
{
    uint32_t now =
        to_ms_since_boot(get_absolute_time());

    for (int i = 0; i < BUTTON_COUNT; i++) {

        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);

        last_raw[i] = !gpio_get(button_pins[i]);
        stable_pressed[i] = last_raw[i];
        last_change_ms[i] = now;
    }
}

/*
 * Detect a new directional button press.
 *
 * Returns DIR_NONE when no new press occurs.
 * Each held button produces only one event.
 */
direction_t buttons_get_press(void)
{
    uint32_t now =
        to_ms_since_boot(get_absolute_time());

    direction_t result = DIR_NONE;

    for (int i = 0; i < BUTTON_COUNT; i++) {

        // Buttons are active LOW
        bool pressed = !gpio_get(button_pins[i]);

        // Detect input change
        if (pressed != last_raw[i]) {
            last_raw[i] = pressed;
            last_change_ms[i] = now;
        }

        // Check debounce interval
        if ((uint32_t)(now - last_change_ms[i])
            >= BUTTON_DEBOUNCE_MS) {

            if (pressed != stable_pressed[i]) {
                stable_pressed[i] = pressed;

                // Only report a new press
                if (pressed && result == DIR_NONE) {
                    result = button_directions[i];
                }
            }
        }
    }

    return result;
}
