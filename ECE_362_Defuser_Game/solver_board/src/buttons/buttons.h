#ifndef BUTTONS_H
#define BUTTONS_H

#define BUTTON_UP_PIN     10
#define BUTTON_DOWN_PIN   11
#define BUTTON_LEFT_PIN   12
#define BUTTON_RIGHT_PIN  13

#define BUTTON_DEBOUNCE_MS 25

typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} direction_t;

// Initialize directional button GPIO pins
void buttons_init(void);

// Return a new button press, or DIR_NONE
direction_t buttons_get_press(void);

#endif
