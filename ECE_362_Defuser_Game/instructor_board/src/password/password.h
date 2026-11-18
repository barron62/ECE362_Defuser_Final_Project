/*
 * ECE 362 - Defuser Game
 * Instructor Board - Password Puzzle
 *
 * Generates and validates a random
 * four-digit password.
 */

#ifndef PASSWORD_H
#define PASSWORD_H

#include <stdint.h>
#include <stdbool.h>

#define PASSWORD_LENGTH 4

void password_init(void);

const uint8_t *password_get_code(void);

bool password_matches(const uint8_t *answer);

#endif
