
/*
 * ECE 362 - HUB75 Direct GPIO Color Test
 *
 * Uses the refresh method that successfully
 * illuminated the entire matrix red.
 *
 * RED -> GREEN -> BLUE -> OFF
 *
 * No repeating timers or framebuffer.
 */

#include <stdint.h>
#include "pico/stdlib.h"

// Current working GPIO assignments
#define PIN_R1   27
#define PIN_G1   28
#define PIN_B1   29

#define PIN_R2   30
#define PIN_G2   31
#define PIN_B2   32

#define PIN_A    33
#define PIN_B    34
#define PIN_C    35
#define PIN_D    38

#define PIN_CLK  39
#define PIN_LAT  13
#define PIN_OE   20

#define MATRIX_WIDTH 64
#define SCAN_ROWS 16

static void select_row(uint8_t row)
{
    gpio_put(PIN_A, (row >> 0) & 1);
    gpio_put(PIN_B, (row >> 1) & 1);
    gpio_put(PIN_C, (row >> 2) & 1);
    gpio_put(PIN_D, (row >> 3) & 1);
}

static void set_rgb(uint8_t color)
{
    // Both halves display the same color
    gpio_put(PIN_R1, (color >> 0) & 1);
    gpio_put(PIN_G1, (color >> 1) & 1);
    gpio_put(PIN_B1, (color >> 2) & 1);

    gpio_put(PIN_R2, (color >> 0) & 1);
    gpio_put(PIN_G2, (color >> 1) & 1);
    gpio_put(PIN_B2, (color >> 2) & 1);
}

static void display_row(uint8_t row, uint8_t color)
{
    // Disable output
    gpio_put(PIN_OE, 1);

    // Set color data
    set_rgb(color);

    // Shift all 64 columns
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
    {
        gpio_put(PIN_CLK, 0);
        sleep_us(1);

        gpio_put(PIN_CLK, 1);
        sleep_us(1);

        gpio_put(PIN_CLK, 0);
    }

    // Latch shifted data
    gpio_put(PIN_LAT, 1);
    sleep_us(1);
    gpio_put(PIN_LAT, 0);

    // Select row
    select_row(row);
    sleep_us(1);

    // Display row
    gpio_put(PIN_OE, 0);
    sleep_us(200);

    // Blank before next row
    gpio_put(PIN_OE, 1);
}

int main(void)
{
    stdio_init_all();

    const uint8_t pins[] = {
        PIN_R1, PIN_G1, PIN_B1,
        PIN_R2, PIN_G2, PIN_B2,
        PIN_A, PIN_B, PIN_C, PIN_D,
        PIN_CLK, PIN_LAT, PIN_OE
    };

    for (uint8_t i = 0;
         i < sizeof(pins) / sizeof(pins[0]);
         i++)
    {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
        gpio_put(pins[i], 0);
    }

    gpio_put(PIN_OE, 1);

    const uint8_t colors[] = {
        1,  // RED
        2,  // GREEN
        4,  // BLUE
        0   // OFF
    };

    uint8_t color_index = 0;
    uint64_t last_change = time_us_64();

    while (true)
    {
        // Change color every 2 seconds
        uint64_t now = time_us_64();

        if (now - last_change >= 2000000)
        {
            color_index = (color_index + 1) % 4;
            last_change = now;
        }

        // Continuously refresh every row pair
        for (uint8_t row = 0; row < SCAN_ROWS; row++)
        {
            display_row(row, colors[color_index]);
        }
    }
}
