/*
 * ECE 362 - Defuser Game
 * Solver Board Password Entry
 *
 * Collects a 16-digit password from the 4x4 keypad.
 *
 * Controls:
 * - 0 through 9: Enter password digits
 * - *: Clear current password
 * - #: Submit completed password
 * - A through D: Ignored
 *
 * Password is not validated locally.
 * Instructor board checks the submitted answer.
 */

#include "password_entry.h"
#include <string.h>

static uint8_t password[PASSWORD_LENGTH];
static uint8_t password_length = 0;
static bool password_ready = false;

void password_init(void)
{
    password_clear();
}

void password_clear(void)
{
    memset(password, 0, sizeof(password));
    password_length = 0;
    password_ready = false;
}

uint8_t password_get_length(void)
{
    return password_length;
}

bool password_is_ready(void)
{
    return password_ready;
}

void password_process_key(char key)
{
    // Do not modify a submitted password
    if (password_ready) {
        return;
    }

    // Clear password entry
    if (key == '*') {
        password_clear();
        return;
    }

    // Submit only when all 16 digits are entered
    if (key == '#') {
        if (password_length == PASSWORD_LENGTH) {
            password_ready = true;
        }
        return;
    }

    // Accept numeric digits only
    if (key >= '0' && key <= '9') {
        if (password_length < PASSWORD_LENGTH) {
            password[password_length] =
                (uint8_t)(key - '0');

            password_length++;
        }
    }
}

bool password_get_answer(uint8_t *answer)
{
    if (!password_ready || answer == NULL) {
        return false;
    }

    memcpy(answer, password, PASSWORD_LENGTH);
    return true;
}
