
/*
 * ECE 362 - Defuser Game
 * Instructor Board - HUB75 Matrix Driver
 *
 * Hardware:
 * - RP2350 Proton
 * - 64x32 HUB75 RGB LED matrix
 * - 1/16 scan, four row address signals
 * - GPIO-driven refresh (no PIO)
 *
 * Display:
 * - Randomly generated maze
 * - Blue walls
 * - Black paths
 * - Green start
 * - Red goal
 * - Yellow player
 *
 * Uses 1-bit RGB color (8 possible colors).
 */

#include "hub75.h"
#include "maze/maze.h"

#include <string.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"

// HUB75 GPIO assignments
#define PIN_R1   0
#define PIN_G1   1
#define PIN_B1   2

#define PIN_R2   3
#define PIN_G2   4
#define PIN_B2   5

#define PIN_A    6
#define PIN_B    7
#define PIN_C    8
#define PIN_D    9

#define PIN_CLK  10
#define PIN_LAT  11
#define PIN_OE   12

// 1/16 scan: 16 row pairs per frame
#define HUB75_SCAN_ROWS 16

// 400 microseconds per row pair (~156 Hz/frame)
#define ROW_INTERVAL_US 400

#define DATA_MASK 0x3Fu
#define ADDR_MASK (0xFu << PIN_A)

// 1-bit RGB color values
#define COLOR_BLACK   0x00
#define COLOR_RED     0x01
#define COLOR_GREEN   0x02
#define COLOR_BLUE    0x04
#define COLOR_YELLOW  0x03

// Two independent framebuffers
static uint8_t framebuffer[2]
    [HUB75_HEIGHT][HUB75_WIDTH];

// Buffer currently being displayed
static volatile uint8_t front_buffer = 0;

// Buffer being prepared by main program
static uint8_t back_buffer = 1;

static repeating_timer_t refresh_timer;
static uint8_t scan_row = 0;
static bool initialized = false;

/*
 * Write one pixel into the back framebuffer.
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

    framebuffer[back_buffer][y][x] = color;
}

/*
 * Swap framebuffers atomically.
 *
 * The timer interrupt always reads the
 * front buffer. Main writes to the back.
 */
static void show_frame(void)
{
    uint32_t irq_state = save_and_disable_interrupts();

    uint8_t old_front = front_buffer;
    front_buffer = back_buffer;
    back_buffer = old_front;

    restore_interrupts(irq_state);
}

/*
 * Scan one pair of HUB75 rows.
 *
 * This runs from the repeating-timer
 * interrupt. Keep it short and avoid
 * TFT, UART, or blocking delays here.
 */
static bool refresh_callback(repeating_timer_t *timer)
{
    (void)timer;

    uint8_t row = scan_row;
    uint8_t active = front_buffer;

    // Blank output while shifting data
    gpio_put(PIN_OE, 1);

    // Select row address A-D
    gpio_put_masked(
        ADDR_MASK,
        ((uint32_t)row << PIN_A)
    );

    // Shift 64 column pairs
    for (uint8_t x = 0; x < HUB75_WIDTH; x++)
    {
        // Upper half: row 0-15
        uint8_t top =
            framebuffer[active][row][x];

        // Lower half: row 16-31
        uint8_t bottom =
            framebuffer[active][row + 16][x];

        // Data pins GP0-GP5 correspond to
        // R1,G1,B1,R2,G2,B2.
        uint32_t bits =
            (uint32_t)(top & 0x07u) |
            ((uint32_t)(bottom & 0x07u) << 3);

        gpio_put_masked(DATA_MASK, bits);

        // Clock one column into panel
        gpio_put(PIN_CLK, 1);
        gpio_put(PIN_CLK, 0);
    }

    // Latch shifted pixel data
    gpio_put(PIN_LAT, 1);
    gpio_put(PIN_LAT, 0);

    // Enable visible output (active low)
    gpio_put(PIN_OE, 0);

    // Advance to next row pair
    scan_row++;

    if (scan_row >= HUB75_SCAN_ROWS)
    {
        scan_row = 0;
    }

    return true;
}

/*
 * Initialize HUB75 GPIO and refresh timer.
 */
bool hub75_init(void)
{
    if (initialized)
    {
        return true;
    }

    // Initialize all HUB75 control/data pins
    for (uint8_t pin = PIN_R1; pin <= PIN_OE; pin++)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }

    // Disable panel output initially
    gpio_put(PIN_OE, 1);

    memset(framebuffer, 0, sizeof(framebuffer));

    front_buffer = 0;
    back_buffer = 1;
    scan_row = 0;

    // Negative interval uses completion-based
    // scheduling with Pico repeating timers.
    if (!add_repeating_timer_us(
            -ROW_INTERVAL_US,
            refresh_callback,
            NULL,
            &refresh_timer))
    {
        return false;
    }

    initialized = true;
    return true;
}

/*
 * Draw a 2x2 pixel maze tile.
 */
static void draw_tile(
    uint8_t tile_x,
    uint8_t tile_y,
    uint8_t color)
{
    // Center 42x22 maze on 64x32 panel
    uint8_t start_x =
        11 + tile_x * MAZE_TILE_SIZE;

    uint8_t start_y =
        5 + tile_y * MAZE_TILE_SIZE;

    for (uint8_t dy = 0; dy < MAZE_TILE_SIZE; dy++)
    {
        for (uint8_t dx = 0; dx < MAZE_TILE_SIZE; dx++)
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
 * Render current maze state.
 *
 * Reads maze data from maze.c.
 * Does not generate a new maze.
 */
void hub75_draw_maze(void)
{
    // Clear back buffer
    memset(
        framebuffer[back_buffer],
        0,
        sizeof(framebuffer[back_buffer])
    );

    for (uint8_t y = 0; y < MAZE_HEIGHT; y++)
    {
        for (uint8_t x = 0; x < MAZE_WIDTH; x++)
        {
            maze_tile_t tile = maze_get_tile(x, y);

            uint8_t color = COLOR_BLACK;

            switch (tile)
            {
                case MAZE_WALL:
                    color = COLOR_BLUE;
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

                case MAZE_PATH:
                default:
                    color = COLOR_BLACK;
                    break;
            }

            draw_tile(x, y, color);
        }
    }

    // Show completed image
    show_frame();
}

/*
 * Clear the matrix display.
 */
void hub75_clear(void)
{
    memset(
        framebuffer[back_buffer],
        0,
        sizeof(framebuffer[back_buffer])
    );

    show_frame();
}
