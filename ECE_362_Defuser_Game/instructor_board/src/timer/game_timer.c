/*
 * ECE 362 - Defuser Game
 * Instructor Board - Game Countdown Timer
 *
 * - Three-minute game timer
 * - Shared across password and maze puzzles
 * - Uses RP2350 system time
 * - Non-blocking countdown
 */

#include "game_timer.h"
#include "pico/stdlib.h"

static uint64_t start_time_us = 0;
static bool timer_started = false;

void game_timer_init(void)
{
    start_time_us = 0;
    timer_started = false;
}

void game_timer_start(void)
{
    start_time_us = time_us_64();
    timer_started = true;
}

uint32_t game_timer_remaining_ms(void)
{
    if (!timer_started)
    {
        return GAME_TIME_MS;
    }

    uint64_t elapsed_ms =
        (time_us_64() - start_time_us) / 1000;

    if (elapsed_ms >= GAME_TIME_MS)
    {
        return 0;
    }

    return GAME_TIME_MS - (uint32_t)elapsed_ms;
}

uint32_t game_timer_remaining_seconds(void)
{
    uint32_t remaining_ms =
        game_timer_remaining_ms();

    // Round up so timer starts at 3:00
    return (remaining_ms + 999) / 1000;
}

bool game_timer_expired(void)
{
    return timer_started &&
           game_timer_remaining_ms() == 0;
}
