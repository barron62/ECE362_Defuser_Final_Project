
/*
 * ECE 362 - Defuser Game
 * Instructor Board - HUB75 Matrix Interface
 *
 * 64x32 RGB LED matrix
 * 1/16 scan, GPIO-driven
 *
 * Displays the randomly generated maze.
 */

#ifndef HUB75_H
#define HUB75_H

#include <stdbool.h>
#include <stdint.h>

#define HUB75_WIDTH 64
#define HUB75_HEIGHT 32

// Initialize GPIO and start matrix refresh
bool hub75_init(void);

// Draw generated maze into matrix framebuffer
void hub75_draw_maze(void);

// Clear the visible matrix
void hub75_clear(void);

#endif
