#ifndef INSTRUCTOR_GAME_CONTROLLER_H
#define INSTRUCTOR_GAME_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

void game_init(void);
void game_register_strike(void);
void game_win(void);
void game_time_expired(void);

uint8_t game_get_strikes(void);
bool game_is_over(void);

#endif
