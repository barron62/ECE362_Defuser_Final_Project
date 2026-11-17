#ifndef PASSWORD_ENTRY_H
#define PASSWORD_ENTRY_H

#include <stdint.h>
#include <stdbool.h>

#define PASSWORD_LENGTH 4

// Initialize password entry
void password_init(void);

// Process a keypad character
void password_process_key(char key);

// Read current password length
uint8_t password_get_length(void);

// Check whether a complete password was submitted
bool password_is_ready(void);

// Copy submitted password into output array
bool password_get_answer(uint8_t *answer);

// Clear password and reset submission state
void password_clear(void);

#endif
