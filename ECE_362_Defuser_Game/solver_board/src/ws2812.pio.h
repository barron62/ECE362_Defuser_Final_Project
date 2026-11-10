
#ifndef WS2812_PIO_H
#define WS2812_PIO_H

#include "hardware/pio.h"
#include "hardware/clocks.h"

// WS2812 timing constants
#define WS2812_T1 3
#define WS2812_T2 3
#define WS2812_T3 4

// PIO instructions for WS2812 LEDs
static const uint16_t ws2812_program_instructions[] = {
    0x6321,
    0x1223,
    0x1200,
    0xa242
};

// Define the PIO program
static const struct pio_program ws2812_program = {
    .instructions = ws2812_program_instructions,
    .length = 4,
    .origin = -1,
};

// Initialize the PIO state machine
static inline void ws2812_program_init(
    PIO pio,
    uint sm,
    uint offset,
    uint pin,
    float freq,
    bool rgbw
) {
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

    pio_sm_config c = pio_get_default_sm_config();

    sm_config_set_wrap(&c, offset, offset + 3);
    sm_config_set_sideset(&c, 1, false, false);
    sm_config_set_sideset_pins(&c, pin);

    sm_config_set_out_shift(&c, false, true, rgbw ? 32 : 24);

    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    int cycles_per_bit =
        WS2812_T1 + WS2812_T2 + WS2812_T3;

    float div = (float)clock_get_hz(clk_sys) / (freq * cycles_per_bit);

    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

#endif
