
/*
 * ECE 362 - Defuser Game
 * SPI TFT Display Driver
 *
 * Display:
 * - 2.2-inch ILI9341 TFT
 * - 240 x 320 resolution
 * - RGB565 color format
 *
 * Hardware:
 * - RP2350 Proton (Instructor Board)
 * - SPI0 SCK:  GP18
 * - SPI0 MOSI: GP19
 * - CS:        GP17
 * - DC/RS:     GP14
 * - RESET:     GP15
 *
 * Features:
 * - TFT initialization
 * - Full-screen color fills
 * - Filled rectangles
 * - Rectangle outlines
 * - 5x7 bitmap text
 * - Scaled characters
 * - Centered text
 */

#include "tft_display.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI configuration
#define TFT_SPI     spi0
#define TFT_SCK     18
#define TFT_MOSI    19
#define TFT_CS      17
#define TFT_DC      14
#define TFT_RST     15

#define TFT_SPI_BAUD 8000000

// ILI9341 commands
#define ILI_SWRESET 0x01
#define ILI_SLPOUT  0x11
#define ILI_NORON   0x13
#define ILI_DISPON  0x29
#define ILI_CASET   0x2A
#define ILI_PASET   0x2B
#define ILI_RAMWR   0x2C
#define ILI_MADCTL  0x36
#define ILI_COLMOD  0x3A

/*
 * Send a command to the TFT.
 */
static void write_command(uint8_t command)
{
    gpio_put(TFT_CS, 0);
    gpio_put(TFT_DC, 0);

    spi_write_blocking(
        TFT_SPI,
        &command,
        1
    );

    gpio_put(TFT_CS, 1);
}

/*
 * Send data to the TFT.
 */
static void write_data(
    const uint8_t *data,
    size_t length)
{
    if (length == 0 || data == NULL)
    {
        return;
    }

    gpio_put(TFT_CS, 0);
    gpio_put(TFT_DC, 1);

    spi_write_blocking(
        TFT_SPI,
        data,
        length
    );

    gpio_put(TFT_CS, 1);
}

/*
 * Write a command followed by data.
 */
static void write_register(
    uint8_t command,
    const uint8_t *data,
    size_t length)
{
    write_command(command);

    if (length > 0)
    {
        write_data(data, length);
    }
}

/*
 * Set the rectangular drawing region.
 */
static void set_window(
    uint16_t x0,
    uint16_t y0,
    uint16_t x1,
    uint16_t y1)
{
    uint8_t column_data[4] =
    {
        (uint8_t)(x0 >> 8),
        (uint8_t)x0,
        (uint8_t)(x1 >> 8),
        (uint8_t)x1
    };

    uint8_t page_data[4] =
    {
        (uint8_t)(y0 >> 8),
        (uint8_t)y0,
        (uint8_t)(y1 >> 8),
        (uint8_t)y1
    };

    write_register(
        ILI_CASET,
        column_data,
        sizeof(column_data)
    );

    write_register(
        ILI_PASET,
        page_data,
        sizeof(page_data)
    );

    write_command(ILI_RAMWR);
}

/*
 * Initialize SPI0 and ILI9341 display.
 */
void tft_init(void)
{
    // Initialize SPI0
    spi_init(TFT_SPI, TFT_SPI_BAUD);

    spi_set_format(
        TFT_SPI,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST
    );

    gpio_set_function(
        TFT_SCK,
        GPIO_FUNC_SPI
    );

    gpio_set_function(
        TFT_MOSI,
        GPIO_FUNC_SPI
    );

    // Configure chip select
    gpio_init(TFT_CS);
    gpio_set_dir(TFT_CS, GPIO_OUT);
    gpio_put(TFT_CS, 1);

    // Configure command/data pin
    gpio_init(TFT_DC);
    gpio_set_dir(TFT_DC, GPIO_OUT);
    gpio_put(TFT_DC, 1);

    // Configure reset pin
    gpio_init(TFT_RST);
    gpio_set_dir(TFT_RST, GPIO_OUT);
    gpio_put(TFT_RST, 1);

    // Hardware reset
    sleep_ms(20);
    gpio_put(TFT_RST, 0);
    sleep_ms(20);
    gpio_put(TFT_RST, 1);
    sleep_ms(150);

    // Software reset
    write_command(ILI_SWRESET);
    sleep_ms(150);

    // Power control initialization
    const uint8_t power_a[] =
        {0x39, 0x2C, 0x00, 0x34, 0x02};
    write_register(0xCB, power_a, sizeof(power_a));

    const uint8_t power_b[] =
        {0x00, 0xC1, 0x30};
    write_register(0xCF, power_b, sizeof(power_b));

    const uint8_t timing_a[] =
        {0x85, 0x00, 0x78};
    write_register(0xE8, timing_a, sizeof(timing_a));

    const uint8_t timing_b[] =
        {0x00, 0x00};
    write_register(0xEA, timing_b, sizeof(timing_b));

    const uint8_t power_seq[] =
        {0x64, 0x03, 0x12, 0x81};
    write_register(0xED, power_seq, sizeof(power_seq));

    const uint8_t pump_ratio[] = {0x20};
    write_register(0xF7, pump_ratio, 1);

    const uint8_t power1[] = {0x23};
    write_register(0xC0, power1, 1);

    const uint8_t power2[] = {0x10};
    write_register(0xC1, power2, 1);

    const uint8_t vcom1[] = {0x3E, 0x28};
    write_register(0xC5, vcom1, 2);

    const uint8_t vcom2[] = {0x86};
    write_register(0xC7, vcom2, 1);

    // Portrait orientation, BGR ordering
    const uint8_t madctl[] = {0x48};
    write_register(ILI_MADCTL, madctl, 1);

    // 16-bit RGB565 color
    const uint8_t color_mode[] = {0x55};
    write_register(ILI_COLMOD, color_mode, 1);

    const uint8_t frame_rate[] = {0x00, 0x18};
    write_register(0xB1, frame_rate, 2);

    const uint8_t display_function[] =
        {0x08, 0x82, 0x27};
    write_register(
        0xB6,
        display_function,
        sizeof(display_function)
    );

    // Exit sleep mode
    write_command(ILI_SLPOUT);
    sleep_ms(150);

    // Normal display operation
    write_command(ILI_NORON);
    sleep_ms(10);

    // Turn display on
    write_command(ILI_DISPON);
    sleep_ms(100);

    // Initial black screen
    tft_fill_screen(TFT_BLACK);
}

/*
 * Fill a rectangular region with a color.
 */
void tft_fill_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint16_t color)
{
    if (width == 0 ||
        height == 0 ||
        x >= TFT_WIDTH ||
        y >= TFT_HEIGHT)
    {
        return;
    }

    // Clip to screen boundaries
    if (width > TFT_WIDTH - x)
    {
        width = TFT_WIDTH - x;
    }

    if (height > TFT_HEIGHT - y)
    {
        height = TFT_HEIGHT - y;
    }

    // Set display drawing region
    set_window(
        x,
        y,
        x + width - 1,
        y + height - 1
    );

    // Prepare 64 RGB565 pixels
    uint8_t buffer[128];

    for (int i = 0; i < 64; i++)
    {
        buffer[2 * i] =
            (uint8_t)(color >> 8);

        buffer[2 * i + 1] =
            (uint8_t)color;
    }

    uint32_t remaining =
        (uint32_t)width * height;

    gpio_put(TFT_CS, 0);
    gpio_put(TFT_DC, 1);

    // Send pixel data using hardware SPI
    while (remaining > 0)
    {
        uint32_t count =
            remaining > 64 ? 64 : remaining;

        spi_write_blocking(
            TFT_SPI,
            buffer,
            count * 2
        );

        remaining -= count;
    }

    gpio_put(TFT_CS, 1);
}

/*
 * Fill the complete display.
 */
void tft_fill_screen(uint16_t color)
{
    tft_fill_rect(
        0,
        0,
        TFT_WIDTH,
        TFT_HEIGHT,
        color
    );
}

/*
 * Draw a rectangular border.
 */
void tft_draw_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint16_t color)
{
    if (width < 2 || height < 2)
    {
        return;
    }

    // Top
    tft_fill_rect(
        x, y, width, 1, color
    );

    // Bottom
    tft_fill_rect(
        x,
        y + height - 1,
        width,
        1,
        color
    );

    // Left
    tft_fill_rect(
        x, y, 1, height, color
    );

    // Right
    tft_fill_rect(
        x + width - 1,
        y,
        1,
        height,
        color
    );
}

/*
 * 5x7 bitmap font.
 *
 * Supports:
 * - A through Z
 * - 0 through 9
 * - Space
 * - ?
 * - #
 *
 * Bit 0 is the top pixel.
 */
static void font_get_char(
    char character,
    uint8_t bitmap[5])
{
    static const uint8_t letters[26][5] =
    {
        {0x7E,0x11,0x11,0x11,0x7E}, // A
        {0x7F,0x49,0x49,0x49,0x36}, // B
        {0x3E,0x41,0x41,0x41,0x22}, // C
        {0x7F,0x41,0x41,0x22,0x1C}, // D
        {0x7F,0x49,0x49,0x49,0x41}, // E
        {0x7F,0x09,0x09,0x09,0x01}, // F
        {0x3E,0x41,0x49,0x49,0x7A}, // G
        {0x7F,0x08,0x08,0x08,0x7F}, // H
        {0x00,0x41,0x7F,0x41,0x00}, // I
        {0x20,0x40,0x41,0x3F,0x01}, // J
        {0x7F,0x08,0x14,0x22,0x41}, // K
        {0x7F,0x40,0x40,0x40,0x40}, // L
        {0x7F,0x02,0x0C,0x02,0x7F}, // M
        {0x7F,0x04,0x08,0x10,0x7F}, // N
        {0x3E,0x41,0x41,0x41,0x3E}, // O
        {0x7F,0x09,0x09,0x09,0x06}, // P
        {0x3E,0x41,0x51,0x21,0x5E}, // Q
        {0x7F,0x09,0x19,0x29,0x46}, // R
        {0x46,0x49,0x49,0x49,0x31}, // S
        {0x01,0x01,0x7F,0x01,0x01}, // T
        {0x3F,0x40,0x40,0x40,0x3F}, // U
        {0x1F,0x20,0x40,0x20,0x1F}, // V
        {0x7F,0x20,0x18,0x20,0x7F}, // W
        {0x63,0x14,0x08,0x14,0x63}, // X
        {0x03,0x04,0x78,0x04,0x03}, // Y
        {0x61,0x51,0x49,0x45,0x43}  // Z
    };

    static const uint8_t digits[10][5] =
    {
        {0x3E,0x51,0x49,0x45,0x3E}, // 0
        {0x00,0x42,0x7F,0x40,0x00}, // 1
        {0x42,0x61,0x51,0x49,0x46}, // 2
        {0x21,0x41,0x45,0x4B,0x31}, // 3
        {0x18,0x14,0x12,0x7F,0x10}, // 4
        {0x27,0x45,0x45,0x45,0x39}, // 5
        {0x3C,0x4A,0x49,0x49,0x30}, // 6
        {0x01,0x71,0x09,0x05,0x03}, // 7
        {0x36,0x49,0x49,0x49,0x36}, // 8
        {0x06,0x49,0x49,0x29,0x1E}  // 9
    };

    // Default to blank
    for (int i = 0; i < 5; i++)
    {
        bitmap[i] = 0;
    }

    // Convert lowercase to uppercase
    if (character >= 'a' &&
        character <= 'z')
    {
        character -= 32;
    }

    if (character >= 'A' &&
        character <= 'Z')
    {
        for (int i = 0; i < 5; i++)
        {
            bitmap[i] =
                letters[character - 'A'][i];
        }
    }
    else if (character >= '0' &&
             character <= '9')
    {
        for (int i = 0; i < 5; i++)
        {
            bitmap[i] =
                digits[character - '0'][i];
        }
    }
    else if (character == '?')
    {
        const uint8_t question[5] =
        {
            0x02,0x01,0x51,0x09,0x06
        };

        for (int i = 0; i < 5; i++)
        {
            bitmap[i] = question[i];
        }
    }
    else if (character == '#')
    {
        const uint8_t hash[5] =
        {
            0x14,0x7F,0x14,0x7F,0x14
        };

        for (int i = 0; i < 5; i++)
        {
            bitmap[i] = hash[i];
        }
    }
}

/*
 * Draw a scaled 5x7 character.
 */
void tft_draw_char(
    uint16_t x,
    uint16_t y,
    char character,
    uint16_t color,
    uint8_t scale)
{
    if (scale == 0)
    {
        return;
    }

    uint8_t bitmap[5];

    font_get_char(character, bitmap);

    for (uint8_t column = 0;
         column < 5;
         column++)
    {
        for (uint8_t row = 0;
             row < 7;
             row++)
        {
            if (bitmap[column] & (1 << row))
            {
                tft_fill_rect(
                    x + column * scale,
                    y + row * scale,
                    scale,
                    scale,
                    color
                );
            }
        }
    }
}

/*
 * Draw text left-aligned.
 */
void tft_draw_text(
    uint16_t x,
    uint16_t y,
    const char *text,
    uint16_t color,
    uint8_t scale)
{
    if (text == NULL || scale == 0)
    {
        return;
    }

    while (*text != '\0')
    {
        tft_draw_char(
            x,
            y,
            *text,
            color,
            scale
        );

        x += 6 * scale;
        text++;
    }
}

/*
 * Draw text centered horizontally.
 */
void tft_draw_text_centered(
    uint16_t y,
    const char *text,
    uint16_t color,
    uint8_t scale)
{
    if (text == NULL || scale == 0)
    {
        return;
    }

    uint16_t length = 0;

    while (text[length] != '\0')
    {
        length++;
    }

    uint32_t text_width =
        (uint32_t)length * 6 * scale;

    uint16_t x = 0;

    if (text_width < TFT_WIDTH)
    {
        x = (TFT_WIDTH - text_width) / 2;
    }

    tft_draw_text(
        x,
        y,
        text,
        color,
        scale
    );
}
