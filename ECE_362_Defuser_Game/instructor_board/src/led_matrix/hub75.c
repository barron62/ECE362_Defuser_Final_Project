
/*
 * ECE 362 - Defuser Game
 * Instructor Board - HUB75 Matrix Driver
 *
 * Hardware:
 * - RP2350 Proton
 * - 64x32 RGB HUB75 LED matrix
 * - 1/16 scan, GPIO-driven
 *
 * Core 0:
 * - Builds maze frames
 * - Updates player position
 * - Requests framebuffer swaps
 *
 * Core 1:
 * - Continuously refreshes HUB75
 * - Uses proven direct-GPIO timing
 * - Displays each row for 200 us
 *
 * Colors:
 * - Blue walls
 * - Black paths
 * - Green start
 * - Red goal
 * - Yellow player
 *
 * Green/blue GPIO mapping corrected for
 * observed panel wiring.
 */

#include "hub75.h"
#include "maze/maze.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

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

#define HUB75_SCAN_ROWS 16
#define ROW_ON_TIME_US 200

// 1-bit RGB logical colors
#define COLOR_BLACK   0x00
#define COLOR_RED     0x01
#define COLOR_GREEN   0x02
#define COLOR_BLUE    0x04
#define COLOR_YELLOW  0x03

// 255 means no framebuffer swap requested
#define NO_SWAP 255

// Two framebuffers
static uint8_t framebuffer[2]
    [HUB75_HEIGHT][HUB75_WIDTH];

// Shared between both cores
static volatile uint8_t front_buffer = 0;
static volatile uint8_t pending_buffer = NO_SWAP;

// Written by Core 0 only
static uint8_t back_buffer = 1;

static bool initialized = false;


/*
 * Set one pixel in the back framebuffer.
 * Called only from Core 0.
 */
static void set_pixel(
    uint8_t x,
    uint8_t y,
    uint8_t color)
{
    if (x >= HUB75_WIDTH ||
        y >= HUB75_HEIGHT)
    {
        return;
    }

    framebuffer[back_buffer][y][x] =
        color & 0x07u;
}


/*
 * Publish completed frame to Core 1.
 *
 * Core 1 swaps buffers at the beginning
 * of a new matrix frame.
 *
 * Core 0 waits until the swap completes
 * before reusing the old front buffer.
 */
static void show_frame(void)
{
    if (!initialized)
    {
        return;
    }

    uint8_t old_front = front_buffer;

    // Ensure framebuffer writes are visible
    __sync_synchronize();

    // Request display of completed frame
    pending_buffer = back_buffer;

    // Wait for Core 1 to accept new frame
    while (pending_buffer != NO_SWAP)
    {
        tight_loop_contents();
    }

    __sync_synchronize();

    // Old front is now safe to modify
    back_buffer = old_front;
}


/*
 * Select one of the 16 row pairs.
 */
static void select_row(uint8_t row)
{
    gpio_put(PIN_A, (row >> 0) & 1);
    gpio_put(PIN_B, (row >> 1) & 1);
    gpio_put(PIN_C, (row >> 2) & 1);
    gpio_put(PIN_D, (row >> 3) & 1);
}


/*
 * Output color data for both halves.
 *
 * Green/blue are swapped to match
 * the physical wiring discovered
 * during standalone color testing.
 */
static void output_rgb(
    uint8_t top,
    uint8_t bottom)
{
    gpio_put(PIN_R1, (top >> 0) & 1);
    gpio_put(PIN_G1, (top >> 2) & 1);
    gpio_put(PIN_B1, (top >> 1) & 1);

    gpio_put(PIN_R2, (bottom >> 0) & 1);
    gpio_put(PIN_G2, (bottom >> 2) & 1);
    gpio_put(PIN_B2, (bottom >> 1) & 1);
}


/*
 * Scan a single row pair.
 *
 * Matches successful direct GPIO test:
 * - Disable output
 * - Shift RGB data
 * - Latch data
 * - Select row
 * - Enable for 200 us
 * - Disable before advancing
 */
static void display_row(
    uint8_t row,
    uint8_t active)
{
    gpio_put(PIN_OE, 1);

    // Shift all 64 columns
    for (uint8_t x = 0; x < HUB75_WIDTH; x++)
    {
        uint8_t top =
            framebuffer[active][row][x];

        uint8_t bottom =
            framebuffer[active][row + 16][x];

        output_rgb(top, bottom);

        gpio_put(PIN_CLK, 0);
        sleep_us(1);

        gpio_put(PIN_CLK, 1);
        sleep_us(1);

        gpio_put(PIN_CLK, 0);
    }

    // Latch row data
    gpio_put(PIN_LAT, 1);
    sleep_us(1);
    gpio_put(PIN_LAT, 0);

    // Select row pair
    select_row(row);
    sleep_us(1);

    // Illuminate row
    gpio_put(PIN_OE, 0);
    sleep_us(ROW_ON_TIME_US);

    // Blank before next row
    gpio_put(PIN_OE, 1);
}


/*
 * Core 1 entry point.
 *
 * Runs forever, independently of Core 0.
 * Displays entire matrix continuously.
 */
static void hub75_core1_main(void)
{
    while (true)
    {
        // Accept completed frames only
        // at frame boundaries.
        uint8_t requested = pending_buffer;

        if (requested != NO_SWAP)
        {
            __sync_synchronize();
            front_buffer = requested;
            __sync_synchronize();
            pending_buffer = NO_SWAP;
        }

        uint8_t active = front_buffer;

        // Refresh all 16 row pairs
        for (uint8_t row = 0;
             row < HUB75_SCAN_ROWS;
             row++)
        {
            display_row(row, active);
        }
    }
}


/*
 * Initialize HUB75 and launch Core 1.
 *
 * Call once from the instructor main().
 */
bool hub75_init(void)
{
    if (initialized)
    {
        return true;
    }

    const uint8_t pins[] =
    {
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

    // Start with LEDs disabled
    gpio_put(PIN_OE, 1);

    memset(framebuffer, 0, sizeof(framebuffer));

    front_buffer = 0;
    back_buffer = 1;
    pending_buffer = NO_SWAP;

    initialized = true;

    // Launch continuous display scanning
    // on the second RP2350 processor core.
    multicore_launch_core1(hub75_core1_main);

    return true;
}


/*
 * Draw a 2x2 pixel maze tile.
 *
 * 21x11 tiles -> 42x22 pixels.
 * Centered on the 64x32 matrix.
 */
static void draw_tile(
    uint8_t tile_x,
    uint8_t tile_y,
    uint8_t color)
{
    uint8_t start_x =
        11 + tile_x * MAZE_TILE_SIZE;

    uint8_t start_y =
        5 + tile_y * MAZE_TILE_SIZE;

    for (uint8_t dy = 0;
         dy < MAZE_TILE_SIZE;
         dy++)
    {
        for (uint8_t dx = 0;
             dx < MAZE_TILE_SIZE;
             dx++)
        {
            set_pixel(
                start_x + dx,
                start_y + dy,
                color
            );
        }
    }
}


/*
 * Draw the current randomized maze.
 *
 * Called after password completion
 * and after valid player movement.
 */
void hub75_draw_maze(void)
{
    if (!initialized)
    {
        return;
    }

    // Clear back framebuffer
    memset(
        framebuffer[back_buffer],
        0,
        sizeof(framebuffer[back_buffer])
    );

    for (uint8_t y = 0; y < MAZE_HEIGHT; y++)
    {
        for (uint8_t x = 0; x < MAZE_WIDTH; x++)
        {
            maze_tile_t tile =
                maze_get_tile(x, y);

            uint8_t color = COLOR_BLACK;

            switch (tile)
            {
                case MAZE_WALL:
                    color = COLOR_BLUE;
                    break;

                case MAZE_PATH:
                    color = COLOR_BLACK;
                    break;

                case MAZE_START:
                    color = COLOR_GREEN;
                    break;

                case MAZE_GOAL:
                    color = COLOR_RED;
                    break;

                case MAZE_PLAYER:
                    color = COLOR_YELLOW;
                    break;

                default:
                    color = COLOR_BLACK;
                    break;
            }

            draw_tile(x, y, color);
        }
    }

    show_frame();
}


/*
 * Clear the complete matrix.
 */
void hub75_clear(void)
{
    if (!initialized)
    {
        return;
    }

    memset(
        framebuffer[back_buffer],
        0,
        sizeof(framebuffer[back_buffer])
    );

    show_frame();
}


/*
 * Full-panel diagnostic color fill.
 *
 * 0 = OFF
 * 1 = RED
 * 2 = GREEN
 * 4 = BLUE
 * 7 = WHITE
 */
void hub75_test_fill(uint8_t color)
{
    if (!initialized)
    {
        return;
    }

    for (uint8_t y = 0;
         y < HUB75_HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < HUB75_WIDTH;
             x++)
        {
            set_pixel(x, y, color);
        }
    }

    show_frame();
}
