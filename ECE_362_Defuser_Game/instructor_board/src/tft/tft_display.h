
#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <stdint.h>

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// RGB565 colors
#define TFT_BLACK       0x0000
#define TFT_WHITE       0xFFFF
#define TFT_RED         0xF800
#define TFT_GREEN       0x07E0
#define TFT_BLUE        0x001F

// Defuser Game colors
#define TFT_BACKGROUND  0x10C3
#define TFT_PANEL       0x1947
#define TFT_BORDER      0x4A69
#define TFT_CYAN        0x3DDF
#define TFT_LIGHT_GRAY  0xCE59
#define TFT_MUTED       0x94B5

void tft_init(void);
void tft_fill_screen(uint16_t color);
void tft_draw_timer(uint32_t seconds_left);
// Graphics
void tft_fill_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint16_t color
);

void tft_draw_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint16_t color
);

// Text
void tft_draw_char(
    uint16_t x,
    uint16_t y,
    char character,
    uint16_t color,
    uint8_t scale
);

void tft_draw_text(
    uint16_t x,
    uint16_t y,
    const char *text,
    uint16_t color,
    uint8_t scale
);

void tft_draw_text_centered(
    uint16_t y,
    const char *text,
    uint16_t color,
    uint8_t scale
);

#endif
