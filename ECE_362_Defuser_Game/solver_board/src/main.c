
/*
 * ECE 362 - Defuser Game
 * Solver Board Main Program
 *
 * UART1:
 * TX = GP36
 * RX = GP37
 *
 * NeoPixel strip = GP16
 *
 * Keypad puzzle:
 * - Enter four digits
 * - Press # to submit
 * - Press * to clear
 * - Instructor validates password
 *
 * Correct answer advances to button puzzle.
 */

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "led/led_strip.h"
#include "uart/uart_solver_v2.h"
#include "game/game_controller.h"
#include "keypad/keypad.h"
#include "buttons/buttons.h"
#include "password/password_entry.h"

#define LED_PIN 16

/*
 * Initialize solver hardware.
 */
static void init_hardware(void)
{
    if (!led_init(LED_PIN))
    {
        while (true)
        {
            tight_loop_contents();
        }
    }

    uart_solver_init();
    game_init();
    keypad_init();
    buttons_init();
    password_init();

    led_running();
}

/*
 * Receive instructor UART commands.
 */
static void process_uart(void)
{
    uint8_t message;

    while (uart_solver_read(&message))
    {
        game_state_t previous_state = game_get_state();
        bool was_pending = game_answer_pending();

        game_process_message(message);

        // Clear submitted password after instructor
        // accepts or rejects the keypad answer.
        if (previous_state == GAME_KEYPAD &&
            was_pending &&
            (message == UART_SUCCESS ||
             message == UART_FAILURE))
        {
            password_clear();
        }

        // Clear old entries when a new game starts
        if (message == UART_GO)
        {
            password_clear();
        }
    }
}

/*
 * Process keypad password submission.
 */
static void process_keypad(void)
{
    char key = keypad_get_key();

    if (key == '\0')
    {
        return;
    }

    // Only accept password keys during keypad puzzle
    if (game_get_state() != GAME_KEYPAD)
    {
        return;
    }

    // Wait until instructor responds before retrying
    if (game_answer_pending())
    {
        return;
    }

    password_process_key(key);

    if (password_is_ready())
    {
        uint8_t answer[PASSWORD_LENGTH];

        if (password_get_answer(answer))
        {
            game_send_answer(answer, PASSWORD_LENGTH);
        }
    }
}

/*
 * Read directional buttons.
 * Communication will be added for the button
 * and maze puzzles.
 */
static void process_buttons(void)
{
    direction_t direction = buttons_get_press();

    switch (direction)
    {
        case DIR_UP:
            // TODO: Send UP movement
            break;

        case DIR_DOWN:
            // TODO: Send DOWN movement
            break;

        case DIR_LEFT:
            // TODO: Send LEFT movement
            break;

        case DIR_RIGHT:
            // TODO: Send RIGHT movement
            break;

        case DIR_NONE:
        default:
            break;
    }
}

/*
 * Trigger LED animations based on game events.
 */
static void handle_led_events(void)
{
    game_event_t event = game_get_event();

    switch (event)
    {
        case GAME_EVENT_STARTED:
            led_running();
            break;

        case GAME_EVENT_PUZZLE_COMPLETE:
            led_puzzle_complete();
            break;

        case GAME_EVENT_WON:
            led_win();
            break;

        case GAME_EVENT_LOST:
            led_game_over();
            break;

        
        case GAME_EVENT_ANSWER_REJECTED:
            // Incorrect password: show red for 500 ms
            led_game_over();

            for (int i = 0; i < 50; i++)
            {
                led_update();
                sleep_ms(10);
            }

            // Return to normal gameplay
            led_running();
            break;


        case GAME_EVENT_NONE:
        default:
            break;
    }
}

/*
 * Main game loop.
 */
int main(void)
{
    init_hardware();

    while (true)
    {
        process_uart();
        process_keypad();
        process_buttons();
        handle_led_events();

        led_update();
        sleep_ms(10);
    }

    return 0;
}
