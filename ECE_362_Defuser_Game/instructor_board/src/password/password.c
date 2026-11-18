/*
 * ECE 362 - Defuser Game
 * Instructor Board - Password Puzzle
 *
 * - Generates a random four-digit password
 * - Allows digits from 0 to 9
 * - Validates submitted passwords
 * - Password changes each game startup
 */
#include <stddef.h>
#include "password.h"
#include "pico/rand.h"

static uint8_t correct_password[PASSWORD_LENGTH];

/*
 * Generate a new random password.
 */
void password_init(void)
{
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        correct_password[i] =
            (uint8_t)(get_rand_32() % 10);
    }
}

/*
 * Return the generated password.
 * Used by the instructor TFT.
 */
const uint8_t *password_get_code(void)
{
    return correct_password;
}

/*
 * Compare submitted password against
 * the generated password.
 */
bool password_matches(const uint8_t *answer)
{
    if (answer == NULL)
    {
        return false;
    }

    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        if (answer[i] != correct_password[i])
        {
            return false;
        }
    }

    return true;
}
