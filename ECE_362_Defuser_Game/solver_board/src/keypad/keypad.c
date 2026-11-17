
/*
 * ECE 362 - Defuser Game
 * Solver Board 4x4 Keypad Driver
 *
 * Uses GPIO matrix scanning with internal pull-ups.
 * Includes software debouncing and one event per press.
 *
 * Key layout:
 *  1  2  3  A
 *  4  5  6  B
 *  7  8  9  C
 *  *  0  #  D
 */

#include "keypad.h"
#include "pico/stdlib.h"
#include <stdint.h>

static const uint row_pins[KEYPAD_ROWS] = {
    KEYPAD_R1, KEYPAD_R2,
    KEYPAD_R3, KEYPAD_R4
};

static const uint col_pins[KEYPAD_COLS] = {
    KEYPAD_C1, KEYPAD_C2,
    KEYPAD_C3, KEYPAD_C4
};

static const char key_map[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Debounce state
static char last_raw = '\0';
static char stable_key = '\0';
static uint32_t last_change_ms = 0;

/*
 * Initialize keypad pins.
 * Rows are inactive inputs until scanned.
 * Columns use internal pull-up resistors.
 */
void keypad_init(void)
{
    for (int i = 0; i < KEYPAD_ROWS; i++) {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], GPIO_IN);
        gpio_disable_pulls(row_pins[i]);
    }

    for (int i = 0; i < KEYPAD_COLS; i++) {
        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], GPIO_IN);
        gpio_pull_up(col_pins[i]);
    }

    last_raw = '\0';
    stable_key = '\0';
    last_change_ms = 0;
}

/*
 * Scan the keypad matrix.
 * Returns the detected key or '\0'.
 *
 * Multiple simultaneous presses are ignored.
 */
static char keypad_scan(void)
{
    char detected = '\0';
    int count = 0;

    for (int row = 0; row < KEYPAD_ROWS; row++) {

        // Drive only the selected row LOW
        gpio_put(row_pins[row], 0);
        gpio_set_dir(row_pins[row], GPIO_OUT);

        sleep_us(5);

        for (int col = 0; col < KEYPAD_COLS; col++) {

            if (!gpio_get(col_pins[col])) {
                detected = key_map[row][col];
                count++;
            }
        }

        // Return row to high impedance
        gpio_set_dir(row_pins[row], GPIO_IN);
    }

    return (count == 1) ? detected : '\0';
}

/*
 * Return a key once per press.
 *
 * Requires the input to remain stable for
 * KEYPAD_DEBOUNCE_MS before reporting it.
 */
char keypad_get_key(void)
{
    char raw = keypad_scan();

    uint32_t now =
        to_ms_since_boot(get_absolute_time());

    if (raw != last_raw) {
        last_raw = raw;
        last_change_ms = now;
    }

    if ((uint32_t)(now - last_change_ms)
        >= KEYPAD_DEBOUNCE_MS) {

        if (raw != stable_key) {
            stable_key = raw;

            if (stable_key != '\0') {
                return stable_key;
            }
        }
    }

    return '\0';
}
