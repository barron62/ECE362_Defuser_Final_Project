#include "pico/stdlib.h"
#include "led_strip.h"

#define LED_PIN 16  // Temporary pin for testing

int main(void) {
    stdio_init_all();

    // Initialize the PIO LED controller
    if (!led_init(LED_PIN)) {
        // Stop here if PIO initialization fails
        while (true) {
            tight_loop_contents();
        }
    }

    // Start with normal green lighting
    led_running();

    uint32_t start_time = to_ms_since_boot(get_absolute_time());

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        uint32_t elapsed = now - start_time;

        // First 5 seconds: normal green
        if (elapsed < 5000) {
            led_running();
        }

        // Next 10 seconds: flashing red warning
        else if (elapsed < 15000) {
            uint32_t time_left = 15000 - elapsed + 5000;
            led_warning(time_left);
        }

        // Trigger puzzle completion once
        else if (elapsed < 16000) {
            led_puzzle_complete();
        }

        // Next 3 seconds: victory animation
        else if (elapsed < 19000) {
            led_win();
        }

        // Final 3 seconds: solid red
        else if (elapsed < 22000) {
            led_game_over();
        }

        // Restart the demonstration
        else {
            start_time = now;
            led_running();
        }

        // Update LEDs using PIO
        led_update();

        sleep_ms(10);
    }
}
