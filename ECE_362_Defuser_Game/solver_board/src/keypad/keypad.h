
#ifndef KEYPAD_H
#define KEYPAD_H

// 4x4 matrix keypad
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4


 // Keypad GPIO assignments
#define KEYPAD_R1 5
#define KEYPAD_R2 4
#define KEYPAD_R3 3
#define KEYPAD_R4 2

#define KEYPAD_C1 9
#define KEYPAD_C2 8
#define KEYPAD_C3 7
#define KEYPAD_C4 6


#define KEYPAD_DEBOUNCE_MS 20

// Initialize keypad GPIO
void keypad_init(void);

// Returns a newly pressed key
// Returns '\0' when no new key is detected
char keypad_get_key(void);

#endif
