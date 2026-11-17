#include "game_controller.h"
#include "uart/uart_solver_v2.h"

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

bool game_send_answer(const uint8_t *data, uint8_t length)
{
    // Answer transmission is not implemented yet.
    // The instructor board must agree on framing
    // and expected payload length first.
    (void)data;
    (void)length;
    return false;
}

void game_process_message(uint8_t message)
{
    switch (message)
    {
        case UART_READY:
            // Instructor requests connection handshake
            uart_solver_send(UART_ACK);
            break;

        case UART_GO:
            // Proposed: begin a new game
            state = GAME_KEYPAD;
            answer_pending = false;
            pending_event = GAME_EVENT_STARTED;
            break;

        case UART_SUCCESS:
            if (state == GAME_KEYPAD) {
                state = GAME_BUTTON;
                pending_event = GAME_EVENT_PUZZLE_COMPLETE;
            }
            else if (state == GAME_BUTTON) {
                state = GAME_MAZE;
                pending_event = GAME_EVENT_PUZZLE_COMPLETE;
            }
            else if (state == GAME_MAZE) {
                state = GAME_WON;
                pending_event = GAME_EVENT_WON;
            }
            answer_pending = false;
            break;

        case UART_FAILURE:
            // Incorrect answer; allow retry
            if (state == GAME_KEYPAD ||
                state == GAME_BUTTON ||
                state == GAME_MAZE) {
                answer_pending = false;
                pending_event = GAME_EVENT_ANSWER_REJECTED;
            }
            break;

        default:
            break;
    }
}
