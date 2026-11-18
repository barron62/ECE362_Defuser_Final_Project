/*
 * ECE 362 - Defuser Game
 * Instructor Board - Temporary Maze Test
 *
 * Expected sequence:
 * UP -> RIGHT -> DOWN -> LEFT
 */

#include "maze.h"

static const uint8_t test_sequence[4] = {
    0, 3, 1, 2
};

static uint8_t sequence_index = 0;

void maze_init(void)
{
    sequence_index = 0;
}

maze_result_t maze_process_direction(uint8_t direction)
{
    // Invalid movement
    if (direction > 3)
    {
        return MAZE_INCORRECT;
    }

    // Incorrect direction: restart sequence
    if (direction != test_sequence[sequence_index])
    {
        sequence_index = 0;
        return MAZE_INCORRECT;
    }

    // Correct direction
    sequence_index++;

    if (sequence_index == 4)
    {
        return MAZE_COMPLETE;
    }

    return MAZE_IN_PROGRESS;
}
