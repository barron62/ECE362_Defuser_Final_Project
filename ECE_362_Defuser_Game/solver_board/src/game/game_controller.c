#include <stddef.h>
#include "game_controller.h"
#include "uart/uart_solver_v2.h"

#define KEYPAD_ANSWER_LENGTH 4

static game_state_t state = GAME_IDLE;
static game_event_t pending_event = GAME_EVENT_NONE;
static bool answer_pending = false;

void game_init(void)
{
    state = GAME_IDLE;
    pending_event = GAME_EVENT_NONE;
    answer_pending = false;
}

game_state_t game_get_state(void)
{
    return state;
}

game_event_t game_get_event(void)
{
    game_event_t event = pending_event;
    pending_event = GAME_EVENT_NONE;
    return event;
}

bool game_answer_pending(void)
{
    return answer_pending;
}

/*
 * Send a four-digit password to the instructor.
 * Packet: START, digit0, digit1, digit2, digit3
 */
bool game_send_answer(const uint8_t *data, uint8_t length)
{
    if (data == NULL ||
        state != GAME_KEYPAD ||
        answer_pending ||
        length != KEYPAD_ANSWER_LENGTH)
    {
        return false;
    }

    // Validate digits before transmitting
    for (uint8_t i = 0; i < length; i++)
    {
        if (data[i] > 9)
        {
            return false;
        }
    }

    answer_pending = true;

    // Start of password message
    uart_solver_send(UART_PASSWORD_SUBMIT);

    // Transmit all four numeric digits
    for (uint8_t i = 0; i < length; i++)
    {
        uart_solver_send(data[i]);
    }

    return true;
}

/*
 * Process instructor control messages.
 */
void game_process_message(uint8_t message)
{
    // Ignore further commands after game over
    if (state == GAME_LOST || state == GAME_WON)
    {
        return;
    }
    switch (message)
    {
        case UART_READY:
            // Instructor requests handshake
            uart_solver_send(UART_ACK);
            break;

        case UART_GO:
            // Begin game with keypad puzzle
            state = GAME_KEYPAD;
            answer_pending = false;
            pending_event = GAME_EVENT_STARTED;
            break;

        
        case UART_SUCCESS:
            if (state == GAME_KEYPAD && answer_pending)
            {
                // Password correct: advance to maze
                state = GAME_MAZE;
                pending_event = GAME_EVENT_PUZZLE_COMPLETE;
                answer_pending = false;
            }
            else if (state == GAME_MAZE)
            {
                // Maze completed: victory
                state = GAME_WON;
                pending_event = GAME_EVENT_WON;
                answer_pending = false;
            }
            break;


        case UART_FAILURE:
            if (state == GAME_KEYPAD && answer_pending)
            {
                answer_pending = false;
                pending_event = GAME_EVENT_ANSWER_REJECTED;
            }
            else if (state == GAME_MAZE)
            {
                answer_pending = false;
                pending_event = GAME_EVENT_ANSWER_REJECTED;
            }
            break;
        
        case UART_GAME_OVER:
            // Instructor ended the game
            state = GAME_LOST;
            answer_pending = false;
            pending_event = GAME_EVENT_LOST;
            break;

        default:
            break;
    }
}
