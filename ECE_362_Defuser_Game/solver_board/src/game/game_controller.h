#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    GAME_IDLE,
    GAME_KEYPAD,
    GAME_BUTTON,
    GAME_MAZE,
    GAME_WON,
    GAME_LOST
} game_state_t;

typedef enum {
    GAME_EVENT_NONE,
    GAME_EVENT_STARTED,
    GAME_EVENT_PUZZLE_COMPLETE,
    GAME_EVENT_WON,
    GAME_EVENT_LOST,
    GAME_EVENT_ANSWER_REJECTED
} game_event_t;

// Initialize game controller
void game_init(void);

// Process received UART control messages
void game_process_message(uint8_t message);

// Read current game state
game_state_t game_get_state(void);

// Retrieve and clear latest game event
game_event_t game_get_event(void);

// Track pending answer acknowledgment
bool game_answer_pending(void);

// Submit a puzzle answer
bool game_send_answer(const uint8_t *data, uint8_t length);

#endif
