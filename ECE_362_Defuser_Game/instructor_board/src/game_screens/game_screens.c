/*
 * ECE 362 - Defuser Game
 * Instructor Board - Game Screen Display
 *
 * - Password puzzle screen
 * - Maze puzzle screen
 * - Shared strike counter
 * - Victory screen
 * - Three-strike game over
 * - Timer expiration screen
 *
 * Uses the SPI TFT display driver.
 */

#include <stdint.h>
#include <stddef.h>
#include "game_screens.h"
#include "tft/tft_display.h"

/*
 * Draw one password digit inside a box.
 */
static void draw_password_box(
    uint16_t x,
    uint8_t digit)
{
    const uint16_t y = 199;
    const uint16_t width = 38;
    const uint16_t height = 50;

    tft_fill_rect(
        x, y, width, height,
        TFT_BACKGROUND
    );

    tft_draw_rect(
        x, y, width, height,
        TFT_BORDER
    );

    tft_draw_char(
        x + 12,
        y + 15,
        (char)('0' + digit),
        TFT_WHITE,
        3
    );
}

/*
 * Display the current strikes.
 * Shows STRIKES 0 OF 3, 1 OF 3, etc.
 */
void game_screen_strikes(uint8_t strikes)
{
    // Clear previous counter
    tft_fill_rect(
        20, 260, 200, 35,
        TFT_PANEL
    );

    tft_draw_text(
        30, 270,
        "STRIKES",
        TFT_LIGHT_GRAY,
        2
    );

    tft_draw_char(
        126, 270,
        (char)('0' + strikes),
        strikes > 0 ? TFT_RED : TFT_GREEN,
        2
    );

    tft_draw_text(
        144, 270,
        "OF 3",
        TFT_LIGHT_GRAY,
        2
    );
}

/*
 * Draw the common game panel and title.
 */
static void draw_game_panel(void)
{
    tft_fill_screen(TFT_BACKGROUND);

    tft_fill_rect(
        8, 12, 224, 296,
        TFT_PANEL
    );

    tft_draw_rect(
        8, 12, 224, 296,
        TFT_BORDER
    );

    tft_draw_text_centered(
        43, "DEFUSER GAME",
        TFT_CYAN, 2
    );

    tft_fill_rect(
        24, 79, 192, 2,
        TFT_BORDER
    );
}

/*
 * Display the randomized password puzzle.
 */
void game_screen_password(const uint8_t *password)
{
    if (password == NULL)
    {
        return;
    }

    draw_game_panel();

    tft_draw_text_centered(
        101, "PUZZLE 1",
        TFT_WHITE, 3
    );

    tft_draw_text_centered(
        157, "PASSWORD CODE",
        TFT_WHITE, 2
    );

    // Display four generated password digits
    draw_password_box(26, password[0]);
    draw_password_box(76, password[1]);
    draw_password_box(126, password[2]);
    draw_password_box(176, password[3]);

    game_screen_strikes(0);
}

/*
 * Display maze puzzle instructions.
 */
void game_screen_maze(void)
{
    draw_game_panel();

    tft_draw_text_centered(
        101, "PUZZLE 2",
        TFT_WHITE, 3
    );

    tft_draw_text_centered(
        157, "MAZE PUZZLE",
        TFT_WHITE, 2
    );

    tft_draw_text_centered(
        206, "GUIDE THE SOLVER",
        TFT_LIGHT_GRAY, 2
    );

    tft_draw_text_centered(
        225, "USE ARROW KEYS",
        TFT_CYAN, 2
    );
}

/*
 * Display victory after completing both puzzles.
 */
void game_screen_victory(void)
{
    tft_fill_screen(TFT_BACKGROUND);

    tft_draw_text_centered(
        120, "BOMB DEFUSED",
        TFT_GREEN, 2
    );
}

/*
 * Display defeat after three strikes.
 */
void game_screen_game_over(void)
{
    tft_fill_screen(TFT_BACKGROUND);

    tft_draw_text_centered(
        110, "GAME OVER",
        TFT_RED, 3
    );

    tft_draw_text_centered(
        175, "THREE STRIKES",
        TFT_WHITE, 2
    );
}

/*
 * Display defeat after timer expiration.
 */
void game_screen_time_expired(void)
{
    tft_fill_screen(TFT_BACKGROUND);

    tft_draw_text_centered(
        110, "TIME EXPIRED",
        TFT_RED, 2
    );

    tft_draw_text_centered(
        175, "GAME OVER",
        TFT_WHITE, 2
    );
}
