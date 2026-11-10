#ifndef LED_STRIP_H
#define LED_STRIP_H

#include <stdint.h>
#include <stdbool.h>

// Number of LEDs on the Adafruit bar
#define NUM_LEDS 8

// Initialize the PIO and LED strip
bool led_init(uint32_t gpio_pin);

// Normal game lighting
void led_running(void);

// Flash faster as time runs out
void led_warning(uint32_t time_remaining_ms);

// Automatically control LEDs based on remaining game time
void led_set_time_remaining(uint32_t time_remaining_ms);

// Animation for completing a puzzle
void led_puzzle_complete(void);

// Animation when the players win
void led_win(void);

// Show that time has expired
void led_game_over(void);

// Update LED animations during gameplay
void led_update(void);

#endif
