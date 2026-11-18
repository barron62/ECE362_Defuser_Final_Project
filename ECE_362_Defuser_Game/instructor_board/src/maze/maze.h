#ifndef MAZE_H
#define MAZE_H

#include <stdint.h>

typedef enum
{
    MAZE_IN_PROGRESS,
    MAZE_INCORRECT,
    MAZE_COMPLETE
} maze_result_t;

void maze_init(void);
maze_result_t maze_process_direction(uint8_t direction);

#endif
