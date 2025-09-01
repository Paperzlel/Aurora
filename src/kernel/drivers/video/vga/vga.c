#include "vga.h"

#include <memory.h>

uint8_t *video_memory = (uint8_t *)0xb8000;

const unsigned SCREEN_WIDTH = 80;
const unsigned SCREEN_HEIGHT = 25;
const unsigned DEFAULT_COLOUR = 0x07;       // change bit 1 for background, bit 2 for foreground

int p_screen_x = 0;
int p_screen_y = 0;

void vga_putchr(char c, int x, int y) {
    video_memory[2 * (y * SCREEN_WIDTH + x)] = c;
}

void vga_putcolour(uint8_t p_colour, int x, int y) {
    video_memory[2 * (y * SCREEN_WIDTH + x) + 1] = p_colour;
}

void vga_scrlscr(int p_amount) {
    for (int i = p_amount; i < SCREEN_HEIGHT; i++) {
        memcpy((uint8_t *)&video_memory[2 * (i - p_amount) * SCREEN_WIDTH], (uint8_t *)&video_memory[2 * i * SCREEN_WIDTH], 2 * SCREEN_WIDTH);
    }

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        vga_putchr('\0', SCREEN_HEIGHT - 1, '\0');
    }
}

void vga_clear() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            vga_putchr('\0', x, y);
            vga_putcolour(DEFAULT_COLOUR, x, y);
        }
    }
}

void vga_write_char(char c) {
    switch (c) {
        case '\n':
            p_screen_x = 0;
            p_screen_y++;
            break;
        case '\r':
            p_screen_x = 0;
            break;
        case '\t':
            for (int i = 0; i < 4 - (p_screen_x % 4); i++) {
                vga_write_char(' ');
            }
            break;
        default:
            vga_putchr(c, p_screen_x, p_screen_y);
            p_screen_x++;
            break;
    }

    if (p_screen_x >= SCREEN_WIDTH) {
        p_screen_x = 0;
        p_screen_y++;
    }

    if (p_screen_y >= SCREEN_HEIGHT) {
        p_screen_y = SCREEN_HEIGHT - 1;
        vga_scrlscr(1);
    }
}
