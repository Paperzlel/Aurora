#include "stdio.h"
#include "x86.h"

uint8_t *video_memory = (uint8_t *)0xb8000;

const unsigned SCREEN_WIDTH = 80;
const unsigned SCREEN_HEIGHT = 25;
const unsigned DEFAULT_COLOUR = 0x07;       // change bit 1 for background, bit 2 for foreground

int p_screen_x = 0;
int p_screen_y = 0;

void putchr(int x, int y, char c) {
    video_memory[2 * (y * SCREEN_WIDTH + x)] = c; // y * scr_width + x is cell offset, mul by two because of the two-byte size
}

void putcolour(int x, int y, uint8_t colour) {
    video_memory[2 * (y * SCREEN_WIDTH + x) + 1] = colour;
}

void clrscr() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            putchr(x, y, '\0');
            putcolour(x, y, DEFAULT_COLOUR);
        }
    }
}

void putc(char c) {
    switch (c) {
        case '\n':
            p_screen_x = 0;
            p_screen_y++;
            break;
        case '\t':
            for (int i = 0; i < 4 - (p_screen_x % 4); i++) {
                putc(' ');
            }
            break;
        case '\r':
            p_screen_x = 0;
        default:
            putchr(p_screen_x, p_screen_y, c);
            p_screen_x++;
            break;
    }
}

void puts(const char *str) {
    while (*str) {
        putc(*str);
        str++;
    }
}