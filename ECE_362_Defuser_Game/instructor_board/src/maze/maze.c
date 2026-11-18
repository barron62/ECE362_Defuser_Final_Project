
/*
 * ECE 362 - Defuser Game
 * Instructor Board - Random Maze Generator
 *
 * Features:
 * - Random maze generated every startup
 * - Randomized depth-first search algorithm
 * - Guaranteed path from start to finish
 * - Player position tracking
 * - Wall collisions count as incorrect moves
 * - Compatible with 64x32 RGB LED matrix
 *
 * Maze dimensions: 21x11 tiles
 * Rendering: 2x2 pixels per tile
 *
 * Start: (1, 1)
 * Goal: (19, 9)
 *
 * Direction values:
 * 0 = UP
 * 1 = DOWN
 * 2 = LEFT
 * 3 = RIGHT
 */

#include "maze.h"
#include "pico/rand.h"

#define START_X 1
#define START_Y 1

#define GOAL_X (MAZE_WIDTH - 2)
#define GOAL_Y (MAZE_HEIGHT - 2)

#define MAX_CELLS \
    (((MAZE_WIDTH - 1) / 2) * \
     ((MAZE_HEIGHT - 1) / 2))

// Maze layout: 0 = wall, 1 = path
static uint8_t maze_grid[MAZE_HEIGHT][MAZE_WIDTH];

// Current player position
static uint8_t player_x = START_X;
static uint8_t player_y = START_Y;

// Maze completion flag
static bool maze_completed = false;

// Directions for maze generation
static const int8_t dx[4] = {0, 0, -2, 2};
static const int8_t dy[4] = {-2, 2, 0, 0};

/*
 * Generate a random solvable maze using
 * iterative randomized depth-first search.
 */
void maze_init(void)
{
    // Initialize all tiles as walls
    for (uint8_t y = 0; y < MAZE_HEIGHT; y++)
    {
        for (uint8_t x = 0; x < MAZE_WIDTH; x++)
        {
            maze_grid[y][x] = 0;
        }
    }

    // Stack for depth-first search
    uint8_t stack_x[MAX_CELLS];
    uint8_t stack_y[MAX_CELLS];

    uint8_t stack_size = 0;

    // Start generation in top-left cell
    maze_grid[START_Y][START_X] = 1;

    stack_x[stack_size] = START_X;
    stack_y[stack_size] = START_Y;
    stack_size++;

    while (stack_size > 0)
    {
        // Current DFS cell
        uint8_t x = stack_x[stack_size - 1];
        uint8_t y = stack_y[stack_size - 1];

        // Find unvisited neighboring cells
        uint8_t available[4];
        uint8_t count = 0;

        for (uint8_t dir = 0; dir < 4; dir++)
        {
            int nx = (int)x + dx[dir];
            int ny = (int)y + dy[dir];

            if (nx > 0 &&
                nx < MAZE_WIDTH - 1 &&
                ny > 0 &&
                ny < MAZE_HEIGHT - 1 &&
                maze_grid[ny][nx] == 0)
            {
                available[count] = dir;
                count++;
            }
        }

        if (count == 0)
        {
            // Dead end: backtrack
            stack_size--;
            continue;
        }

        // Choose a random unvisited neighbor
        uint8_t random_index =
            (uint8_t)(get_rand_32() % count);

        uint8_t dir = available[random_index];

        int nx = (int)x + dx[dir];
        int ny = (int)y + dy[dir];

        // Carve wall between the two cells
        int wall_x = ((int)x + nx) / 2;
        int wall_y = ((int)y + ny) / 2;

        maze_grid[wall_y][wall_x] = 1;

        // Mark the new cell as a path
        maze_grid[ny][nx] = 1;

        // Continue DFS from new cell
        stack_x[stack_size] = (uint8_t)nx;
        stack_y[stack_size] = (uint8_t)ny;
        stack_size++;
    }

    // Reset player state
    player_x = START_X;
    player_y = START_Y;
    maze_completed = false;
}

/*
 * Process one directional movement.
 *
 * Returns:
 * MAZE_IN_PROGRESS - valid movement
 * MAZE_INCORRECT   - wall collision
 * MAZE_COMPLETE    - reached goal
 */
maze_result_t maze_process_direction(uint8_t direction)
{
    if (maze_completed)
    {
        return MAZE_COMPLETE;
    }

    int next_x = player_x;
    int next_y = player_y;

    switch (direction)
    {
        case 0: // UP
            next_y--;
            break;

        case 1: // DOWN
            next_y++;
            break;

        case 2: // LEFT
            next_x--;
            break;

        case 3: // RIGHT
            next_x++;
            break;

        default:
            return MAZE_INCORRECT;
    }

    // Check matrix boundaries
    if (next_x < 0 ||
        next_x >= MAZE_WIDTH ||
        next_y < 0 ||
        next_y >= MAZE_HEIGHT)
    {
        return MAZE_INCORRECT;
    }

    // Check for wall collision
    if (maze_grid[next_y][next_x] == 0)
    {
        return MAZE_INCORRECT;
    }

    // Valid movement: update position
    player_x = (uint8_t)next_x;
    player_y = (uint8_t)next_y;

    // Check whether player reached goal
    if (player_x == GOAL_X &&
        player_y == GOAL_Y)
    {
        maze_completed = true;
        return MAZE_COMPLETE;
    }

    return MAZE_IN_PROGRESS;
}

/*
 * Return the tile at a specified coordinate.
 * Used by the LED matrix rendering module.
 */
maze_tile_t maze_get_tile(uint8_t x, uint8_t y)
{
    if (x >= MAZE_WIDTH || y >= MAZE_HEIGHT)
    {
        return MAZE_WALL;
    }

    // Draw player over underlying path
    if (x == player_x && y == player_y)
    {
        return MAZE_PLAYER;
    }

    if (x == START_X && y == START_Y)
    {
        return MAZE_START;
    }

    if (x == GOAL_X && y == GOAL_Y)
    {
        return MAZE_GOAL;
    }

    if (maze_grid[y][x] == 0)
    {
        return MAZE_WALL;
    }

    return MAZE_PATH;
}

/*
 * Return the current player coordinates.
 */
uint8_t maze_get_player_x(void)
{
    return player_x;
}

uint8_t maze_get_player_y(void)
{
    return player_y;
}
