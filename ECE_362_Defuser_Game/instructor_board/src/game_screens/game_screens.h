/*
 * ECE 362 - Defuser Game
 * Instructor Board - Game Screen Interface
 *
 * Handles TFT screen layouts for:
 * - Password puzzle
 * - Maze puzzle
 * - Strike counter
 * - Victory and game-over screens
 */

#ifndef GAME_SCREENS_H
#define GAME_SCREENS_H

#include <stdint.h>

// Display password puzzle with four generated digits
void game_screen_password(const uint8_t *password);

// Display maze puzzle instructions
void game_screen_maze(void);

// Update the shared strike counter
void game_screen_strikes(uint8_t strikes);

// Display victory screen
void game_screen_victory(void);

// Display game over after three strikes
void game_screen_game_over(void);

// Display game over when time expires
void game_screen_time_expired(void);

#endif
