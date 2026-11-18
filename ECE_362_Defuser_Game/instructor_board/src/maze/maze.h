
/*
 * ECE 362 - Defuser Game
 * Instructor Board - Random Maze Interface
 *
 * - Random maze generation
 * - Guaranteed solvable path
 * - Player movement validation
 * - Wall collision detection
 * - Goal detection
 * - Maze data for 64x32 LED matrix
 */

#ifndef MAZE_H
#define MAZE_H

#include <stdint.h>

#define MAZE_WIDTH 21
#define MAZE_HEIGHT 11

// Each maze tile occupies 2x2 LED pixels
#define MAZE_TILE_SIZE 2

typedef enum
{
    MAZE_IN_PROGRESS,
    MAZE_INCORRECT,
    MAZE_COMPLETE
} maze_result_t;

typedef enum
{
    MAZE_WALL,
    MAZE_PATH,
    MAZE_START,
    MAZE_GOAL,
    MAZE_PLAYER
} maze_tile_t;

// Generate a new random maze
void maze_init(void);

// Process directional movement over UART
// 0 = UP, 1 = DOWN, 2 = LEFT, 3 = RIGHT
maze_result_t maze_process_direction(uint8_t direction);

// Read maze tile for LED matrix display
maze_tile_t maze_get_tile(uint8_t x, uint8_t y);

// Get current player position
uint8_t maze_get_player_x(void);
uint8_t maze_get_player_y(void);

#endif
