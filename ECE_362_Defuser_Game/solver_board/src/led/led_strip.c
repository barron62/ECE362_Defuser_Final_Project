/*
 * ECE 362 - Defuser Game
 * PIO LED Strip Controller - Solver Board
 *
 * Hardware:
 * - RP2350 microcontroller using PIO0
 * - 8 individually addressable NeoPixel RGB LEDs
 * - WS2812 protocol at 800 kHz
 * - GPIO16 currently assigned for LED data (temporary)
 *
 * LED States:
 * RUNNING:
 *   > 30 seconds: Solid green
 *   15-30 seconds: Red flashing every 800 ms
 *   5-15 seconds: Red flashing every 250 ms
 *   0-5 seconds: Red flashing every 100 ms
 *
 * PUZZLE_COMPLETE:
 *   - Green light moves across all 8 LEDs
 *   - Animation lasts 1000 ms
 *   - Automatically returns to RUNNING
 *
 * WIN:
 *   - Continuous moving green light
 *   - Advances one LED every 100 ms
 *
 * GAME_OVER:
 *   - Solid red when the timer reaches zero
 *
 * State Priority:
 *   1. WIN / GAME_OVER - Terminal states
 *   2. PUZZLE_COMPLETE - Temporary animation
 *   3. RUNNING - Normal and countdown lighting
 *
 * Timing:
 * - PIO transmits WS2812 data at 800 kHz
 * - LED frames update approximately every 30 ms
 * - 80 us reset/latch delay after each frame
 * - Initial 60-second countdown is temporary
 *
 * Integration:
 * - led_init(): Initialize PIO hardware
 * - led_running(): Reset LEDs for a new game
 * - led_set_time_remaining(): Update remaining time
 * - led_warning(): Compatibility with older test code
 * - led_puzzle_complete(): Trigger completion animation
 * - led_win(): Trigger victory animation
 * - led_game_over(): Trigger failure indication
 * - led_update(): Refresh LEDs from the main game loop
 *
 * PIO handles the precise LED signal timing while
 * the CPU manages game states and animations.
 *
 * Hardware testing pending.
 */

#include "led_strip.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

#define LED_FREQ 800000
#define BRIGHTNESS 40

#define WARNING_START_MS 30000
#define FAST_WARNING_MS 15000
#define CRITICAL_WARNING_MS 5000
#define CELEBRATION_MS 1000
#define FRAME_INTERVAL_MS 30

static PIO led_pio = pio0;
static uint led_sm;

static bool initialized = false;
static uint32_t time_left = 60000;                                   // Temporary 60s initial value
static uint32_t animation_start = 0;
static uint32_t last_update = 0;

// LED animation states
typedef enum {
    RUNNING,
    PUZZLE_COMPLETE,
    WIN,
    GAME_OVER
} led_state_t;

static led_state_t state = RUNNING;

// Convert RGB to the GRB format used by NeoPixels
static uint32_t rgb(uint8_t red,
                    uint8_t green,
                    uint8_t blue) {
    return ((uint32_t)green << 16) |
           ((uint32_t)red << 8) |
           (uint32_t)blue;
}

// Send one LED color through PIO
static void send_pixel(uint32_t color) {
    pio_sm_put_blocking(led_pio, led_sm, color << 8);
}

// Set all eight LEDs to the same color
static void set_all(uint8_t red,
                    uint8_t green,
                    uint8_t blue) {
    uint32_t color = rgb(red, green, blue);

    for (int i = 0; i < NUM_LEDS; i++) {
        send_pixel(color);
    }
}

// Illuminate only one LED
static void set_one(int position,
                    uint8_t red,
                    uint8_t green,
                    uint8_t blue) {
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i == position) {
            send_pixel(rgb(red, green, blue));
        } else {
            send_pixel(0);
        }
    }
}

// Initialize the PIO state machine
bool led_init(uint32_t gpio_pin) {
    if (initialized) return true;

    if (!pio_can_add_program(led_pio, &ws2812_program)) {
        return false;
    }

    int sm = pio_claim_unused_sm(led_pio, false);
    if (sm < 0) return false;

    led_sm = (uint)sm;

    uint offset =
        pio_add_program(led_pio, &ws2812_program);

    ws2812_program_init(
        led_pio,
        led_sm,
        offset,
        gpio_pin,
        LED_FREQ,
        false
    );

    initialized = true;
    return true;
}

// Reset LEDs for a new game
void led_running(void) {
    state = RUNNING;
    time_left = 60000;                                              // Temporary 60s initial value
}

// Automatically manage warnings from the countdown
void led_set_time_remaining(uint32_t remaining_ms) {
    // Don't change a completed game
    if (state == WIN || state == GAME_OVER) {
        return;
    }

    time_left = remaining_ms;

    // Time expiration overrides puzzle animations
    if (time_left == 0) {
        state = GAME_OVER;
    }
}

// Keep the old function for test compatibility
void led_warning(uint32_t remaining_ms) {
    led_set_time_remaining(remaining_ms);
}

// Puzzle completion: one-second green animation
void led_puzzle_complete(void) {
    if (state != RUNNING) return;

    state = PUZZLE_COMPLETE;
    animation_start =
        to_ms_since_boot(get_absolute_time());
}

// Victory overrides running and puzzle effects
void led_win(void) {
    if (state == GAME_OVER) return;
    state = WIN;
}

// Game over overrides running and puzzle effects
void led_game_over(void) {
    if (state == WIN) return;
    state = GAME_OVER;
}

// Determine flash speed based on remaining time
static uint32_t warning_interval(void) {
    if (time_left > FAST_WARNING_MS) {
        return 800;
    } else if (time_left > CRITICAL_WARNING_MS) {
        return 250;
    } else {
        return 100;
    }
}

// Update LEDs without long animation delays
void led_update(void) {
    if (!initialized) return;

    uint32_t now =
        to_ms_since_boot(get_absolute_time());

    // Update approximately every 30 ms
    if ((uint32_t)(now - last_update) <
        FRAME_INTERVAL_MS) {
        return;
    }

    last_update = now;

    // Check timer expiration first
    if (time_left == 0 && state != WIN) {
        state = GAME_OVER;
    }

    // Handle puzzle completion animation
    if (state == PUZZLE_COMPLETE) {
        uint32_t elapsed = now - animation_start;

        if (elapsed < CELEBRATION_MS) {
            int position =
                (elapsed / 125) % NUM_LEDS;

            set_one(position, 0, BRIGHTNESS, 0);
            sleep_us(80);
            return;
        }

        // Automatically resume countdown lighting
        state = RUNNING;
    }

    switch (state) {

        case RUNNING: {
            if (time_left > WARNING_START_MS) {
                // More than 30 seconds: green
                set_all(0, BRIGHTNESS, 0);
            } else {
                // Flash red, faster as time runs out
                uint32_t interval =
                    warning_interval();

                if ((now / interval) % 2 == 0) {
                    set_all(BRIGHTNESS, 0, 0);
                } else {
                    set_all(0, 0, 0);
                }
            }
            break;
        }

        case WIN: {
            // Moving green victory animation
            int position =
                (now / 100) % NUM_LEDS;

            set_one(position, 0, BRIGHTNESS, 0);
            break;
        }

        case GAME_OVER:
            // Solid red
            set_all(BRIGHTNESS, 0, 0);
            break;

        case PUZZLE_COMPLETE:
            // Already handled above
            break;
    }

    sleep_us(80);
}
