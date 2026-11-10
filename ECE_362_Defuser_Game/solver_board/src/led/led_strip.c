#include "led_strip.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

#define LED_FREQ 800000
#define BRIGHTNESS 40

static PIO led_pio = pio0;
static uint led_sm;

static bool initialized = false;
static uint32_t time_left = 60000;
static uint32_t animation_start = 0;
static uint32_t last_update = 0;

typedef enum {
    RUNNING,
    WARNING,
    PUZZLE_COMPLETE,
    WIN,
    GAME_OVER
} led_state_t;

static led_state_t state = RUNNING;
static led_state_t previous_state = RUNNING;

// Convert RGB values to the GRB format used by NeoPixels
static uint32_t rgb(uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint32_t)green << 16) |
           ((uint32_t)red << 8) |
           (uint32_t)blue;
}

// Send one LED color through the PIO FIFO
static void send_pixel(uint32_t color) {
    pio_sm_put_blocking(led_pio, led_sm, color << 8);
}

// Set all eight LEDs
static void set_all(uint8_t red, uint8_t green, uint8_t blue) {
    uint32_t color = rgb(red, green, blue);

    for (int i = 0; i < NUM_LEDS; i++) {
        send_pixel(color);
    }
}

// Light only one LED
static void set_one(int position, uint8_t red,
                    uint8_t green, uint8_t blue) {
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

    uint offset = pio_add_program(led_pio, &ws2812_program);

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

// Normal gameplay: solid green
void led_running(void) {
    if (state == RUNNING) return;
    state = RUNNING;
}

// Countdown warning: flashing red
void led_warning(uint32_t time_remaining_ms) {
    time_left = time_remaining_ms;

    // Preserve the puzzle completion animation
    if (state == PUZZLE_COMPLETE) {
        previous_state = WARNING;
        return;
    }

    if (state != GAME_OVER && state != WIN) {
        state = WARNING;
    }
}

// Puzzle solved: green moving light
void led_puzzle_complete(void) {
    if (state == WIN || state == GAME_OVER) return;
    if (state == PUZZLE_COMPLETE) return;

    previous_state = state;
    state = PUZZLE_COMPLETE;
    animation_start = to_ms_since_boot(get_absolute_time());
}

// All puzzles solved: victory animation
void led_win(void) {
    state = WIN;
}

// Timer expired: solid red
void led_game_over(void) {
    state = GAME_OVER;
}

// Update animation without long delays
void led_update(void) {
    if (!initialized) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Send a new frame approximately every 30 ms
    if ((uint32_t)(now - last_update) < 30) return;
    last_update = now;

    switch (state) {

        case RUNNING:
            set_all(0, BRIGHTNESS, 0);
            break;

        case WARNING: {
            uint32_t interval;

            if (time_left > 30000) {
                interval = 800;
            } else if (time_left > 15000) {
                interval = 500;
            } else if (time_left > 5000) {
                interval = 250;
            } else {
                interval = 100;
            }

            if ((now / interval) % 2 == 0) {
                set_all(BRIGHTNESS, 0, 0);
            } else {
                set_all(0, 0, 0);
            }
            break;
        }

        case PUZZLE_COMPLETE: {
            uint32_t elapsed = now - animation_start;

            if (elapsed >= 1000) {
                state = previous_state;
                break;
            }

            int position = (elapsed / 125) % NUM_LEDS;
            set_one(position, 0, BRIGHTNESS, 0);
            break;
        }

        case WIN: {
            int position = (now / 100) % NUM_LEDS;
            set_one(position, 0, BRIGHTNESS, 0);
            break;
        }

        case GAME_OVER:
            set_all(BRIGHTNESS, 0, 0);
            break;
    }

    // Give the WS2812 LEDs time to latch the frame
    sleep_us(80);
}
