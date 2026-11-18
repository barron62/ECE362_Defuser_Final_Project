#ifndef GAME_TIMER_H
#define GAME_TIMER_H

#include <stdint.h>
#include <stdbool.h>

#define GAME_TIME_MS 180000  // 3 minutes

void game_timer_init(void);
void game_timer_start(void);

uint32_t game_timer_remaining_ms(void);
uint32_t game_timer_remaining_seconds(void);

bool game_timer_expired(void);

#endif
