/*
 * ECE 362 - Defuser Game
 * Instructor Board - Game Controller
 *
 * Tracks shared strikes across both puzzles
 * and handles three-strike game over.
 */

#include "game_controller.h"

#include "uart/uart_instructor_v2.h"
#include "game_screens/game_screens.h"

#define MAX_STRIKES 3

static uint8_t strikes = 0;
static bool game_over = false;

void game_init(void)
{
    strikes = 0;
    game_over = false;
}

void game_register_strike(void)
{
    if (game_over)
    {
        return;
    }

    strikes++;

    game_screen_strikes(strikes);

    if (strikes >= MAX_STRIKES)
    {
        game_over = true;
        uart_instructor_send(UART_GAME_OVER);
        game_screen_game_over();
    }
    else
    {
        uart_instructor_send(UART_FAILURE);
    }
}

uint8_t game_get_strikes(void)
{
    return strikes;
}

bool game_is_over(void)
{
    return game_over;
}


/*
 * End the game successfully.
 */
void game_win(void)
{
    if (game_over)
    {
        return;
    }

    game_over = true;

    uart_instructor_send(UART_SUCCESS);
    game_screen_victory();
}

/*
 * End the game when time expires.
 */
void game_time_expired(void)
{
    if (game_over)
    {
        return;
    }

    game_over = true;

    uart_instructor_send(UART_GAME_OVER);
    game_screen_time_expired();
}
