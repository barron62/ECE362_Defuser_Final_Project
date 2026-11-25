
// /*
//  * ECE 362 - Defuser Game
//  * Instructor Board - HUB75 Matrix Interface
//  *
//  * 64x32 RGB LED matrix
//  * 1/16 scan, GPIO-driven
//  *
//  * Displays the randomly generated maze.
//  */

// #ifndef HUB75_H
// #define HUB75_H

// #include <stdbool.h>
// #include <stdint.h>

// #define HUB75_WIDTH 64
// #define HUB75_HEIGHT 32

// // Initialize GPIO and start matrix refresh
// bool hub75_init(void);

// // Draw generated maze into matrix framebuffer
// void hub75_draw_maze(void);

// // Clear the visible matrix
// void hub75_clear(void);
// // Fill all 2048 LEDs with one RGB color
// void hub75_test_fill(uint8_t color);
// #endif

/*
 * ECE 362 - Defuser Game
 * Instructor Board - HUB75 Matrix Interface
 *
 * 64x32 RGB LED matrix
 * 1/16 scan, GPIO-driven
 */

#ifndef HUB75_H
#define HUB75_H

#include <stdbool.h>
#include <stdint.h>

#define HUB75_WIDTH 64
#define HUB75_HEIGHT 32

// Initialize GPIO and start matrix refresh
bool hub75_init(void);

// Draw generated maze
void hub75_draw_maze(void);

// Clear matrix
void hub75_clear(void);

// Fill all LEDs with an RGB test color
// 0 = OFF, 1 = RED, 2 = GREEN,
// 4 = BLUE, 7 = WHITE
void hub75_test_fill(uint8_t color);

#endif
