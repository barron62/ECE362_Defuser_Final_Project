/*
 * ECE 362 - Defuser Game
 * Instructor Board Main Program
 *
 * Responsibilities:
 * - SPI TFT display for instructions and maze
 * - HUB75 LED matrix for timer and lives
 * - UART communication with solver board
 *
 * TODO: Integrate instructor-board peripherals
 */

#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();

    while (true) {
        // TODO: Update TFT display
        // TODO: Update HUB75 LED matrix
        // TODO: Process UART communication

        sleep_ms(10);
    }
}
