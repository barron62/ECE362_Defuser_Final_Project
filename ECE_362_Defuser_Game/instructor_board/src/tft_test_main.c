
/*
 * ECE 362 - Defuser Game
 * TFT Puzzle 1 Instruction Screen
 *
 * Display: 240x320 ILI9341
 * SPI0 on instructor Proton
 *
 * Screen:
 * - Dark background
 * - Blue DEFUSER GAME heading
 * - PUZZLE 1
 * - ENTER THE CODE
 * - Four password placeholders
 * - PRESS # TO SUBMIT
 */

#include "pico/stdlib.h"
#include "tft/tft_display.h"

/*
 * Draw a single password placeholder.
 */
static void draw_password_box(uint16_t x)
{
    const uint16_t y = 199;
    const uint16_t width = 38;
    const uint16_t height = 50;

    // Password box background
    tft_fill_rect(
        x, y, width, height,
        TFT_BACKGROUND
    );

    // Box outline
    tft_draw_rect(
        x, y, width, height,
        TFT_BORDER
    );

    // Question mark centered in box
    tft_draw_char(
        x + 12,
        y + 15,
        '?',
        TFT_MUTED,
        3
    );
}

/*
 * Draw the main keypad puzzle screen.
 */
static void draw_keypad_screen(void)
{
    // Dark background
    tft_fill_screen(TFT_BACKGROUND);

    // Main panel
    tft_fill_rect(
        8, 12, 224, 296,
        TFT_PANEL
    );

    // Panel border
    tft_draw_rect(
        8, 12, 224, 296,
        TFT_BORDER
    );

    // Top game heading
    tft_draw_text_centered(
        43,
        "DEFUSER GAME",
        TFT_CYAN,
        2
    );

    // Divider underneath title
    tft_fill_rect(
        24, 79, 192, 2,
        TFT_BORDER
    );

    // Current puzzle
    tft_draw_text_centered(
        101,
        "PUZZLE 1",
        TFT_WHITE,
        3
    );

    // Instructions
    tft_draw_text_centered(
        157,
        "ENTER THE CODE",
        TFT_WHITE,
        2
    );

    // Four password boxes
    draw_password_box(26);
    draw_password_box(76);
    draw_password_box(126);
    draw_password_box(176);

    // Bottom instruction
    tft_draw_text_centered(
        279,
        "PRESS # TO SUBMIT",
        TFT_LIGHT_GRAY,
        2
    );
}

int main(void)
{
    // Initialize TFT hardware
    tft_init();

    // Draw instruction screen once
    draw_keypad_screen();

    // Hold screen indefinitely
    while (true)
    {
        sleep_ms(1000);
    }

    return 0;
}
