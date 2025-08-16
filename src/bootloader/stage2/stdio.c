#include "stdio.h"
#include "x86.h"
#include "memory.h"

// Included from GCC's freestanding library, as implementing them ourselves is a pain
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t *video_memory = (uint8_t *)0xb8000;

const unsigned SCREEN_WIDTH = 80;
const unsigned SCREEN_HEIGHT = 25;
const unsigned DEFAULT_COLOUR = 0x07;       // change bit 1 for background, bit 2 for foreground

#define FB_COMMAND_PORT     0x3d4
#define FB_DATA_PORT        0x3d5

int p_screen_x = 0;
int p_screen_y = 0;

void movecursor(int x, int y) {
    uint16_t res = (y * SCREEN_WIDTH + x);

    x86_outb(FB_COMMAND_PORT, 14);
    x86_outb(FB_DATA_PORT, (res >> 8) & 0xff);
    x86_outb(FB_COMMAND_PORT, 15);
    x86_outb(FB_DATA_PORT, res & 0x00ff);
}

void scroll_screen(uint32_t p_amount) {
    for (int i = p_amount; i < SCREEN_HEIGHT; i++) {
        memcpy((uint8_t *)&video_memory[2 * (i - p_amount) * SCREEN_WIDTH], (uint8_t *)&video_memory[2 * i * SCREEN_WIDTH], 2 * SCREEN_WIDTH);
    }

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        putchr(x, SCREEN_HEIGHT - 1, '\0');
    }
}

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

    if (p_screen_y >= SCREEN_HEIGHT) {
        p_screen_y = SCREEN_HEIGHT - 1;
        scroll_screen(1);
    }

    movecursor(p_screen_x, p_screen_y);
}

void puts(const char *str) {
    while (*str) {
        putc(*str);
        str++;
    }
}

static bool a_is_captial = false;
const char a_hex_lowercase[] = "0123456789abcdef";
const char a_hex_uppercase[] = "0123456789ABCDEF";

void print_num_unsigned(uint64_t number, int base) {
    char buf[32]; // Shouldn't be bigger than this
    int pos = 0;

    do {
        uint64_t rem = number % base;
        number /= base;
        buf[pos++] = a_is_captial ? a_hex_uppercase[rem] : a_hex_lowercase[rem];
    } while (number > 0);

    while (--pos >= 0) {
        putc(buf[pos]);
    }
}

void print_num_signed(int64_t number, int base) {
    if (number < 0) {
        putc('-');
        print_num_unsigned(-number, base);
    } else {
        print_num_unsigned(number, base);
    }
}

typedef enum {
    PRINTF_STATE_NORMAL,
    PRINTF_STATE_IDENTIFIER,
    PRINTF_STATE_LENGTH_SHORT,
    PRINTF_STATE_LENGTH_LONG,
    PRINTF_STATE_NUMBER,
} PrintfState;

typedef enum {
    LENGTH_SHORT_SHORT,
    LENGTH_SHORT,
    LENGTH_DEFAULT,
    LENGTH_LONG,
    LENGTH_LONG_LONG,
} FmtLength;


void printf(const char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);

    PrintfState state = PRINTF_STATE_NORMAL;
    FmtLength length = LENGTH_DEFAULT;
    int base = 10;
    bool number = false;
    bool is_signed = false;

    while (*fmt) {
        
        switch (state) {
            case PRINTF_STATE_NORMAL: {
                switch (*fmt) {
                    case '%':
                        state = PRINTF_STATE_IDENTIFIER;
                        break;
                    default:
                        putc(*fmt);
                        break;
                }
            } break;

            case PRINTF_STATE_LENGTH_SHORT: {
                if (*fmt == 'h') {
                    length = LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_NUMBER;
                } else {
                    goto PRINTF_STATE_IDENTIFIER_;
                }
            } break;


            case PRINTF_STATE_LENGTH_LONG: {
                if (*fmt == 'l') {
                    length = LENGTH_LONG_LONG;
                    state = PRINTF_STATE_IDENTIFIER;
                    break;
                } else {
                    goto PRINTF_STATE_IDENTIFIER_;
                }
            }

            case PRINTF_STATE_IDENTIFIER:
            PRINTF_STATE_IDENTIFIER_:
                switch (*fmt) {
                    case '%':
                        putc(*fmt);
                        state = PRINTF_STATE_NORMAL;
                        break;
                    case 'c':
                        putc((char)va_arg(arg_ptr, int));
                        state = PRINTF_STATE_NORMAL;
                        break;
                    case 's':
                        puts(va_arg(arg_ptr, const char *));
                        state = PRINTF_STATE_NORMAL;
                        break;
                    case 'h':
                        state = PRINTF_STATE_LENGTH_SHORT;
                        fmt++;
                        continue;
                    case 'l':
                        state = PRINTF_STATE_LENGTH_LONG;
                        fmt++;
                        continue;                   // Since we print numbers after this point, increment the pointer and move on immediately
                    case 'd':
                    case 'i':
                        base = 10;
                        is_signed = true;
                        number = true;
                        break;
                    case 'u':
                        base = 10;
                        is_signed = false;
                        number = true;
                        break;
                    case 'x':
                    case 'p':
                        a_is_captial = false;
                        base = 16;
                        is_signed = false;
                        number = true;
                        break;
                    case 'X':
                        a_is_captial = true;
                        base = 16;
                        is_signed = false;
                        number = true;
                        break;
                    case 'o':
                        base = 8;
                        is_signed = false;
                        number = true;
                        break;
                    default:
                        break;      // Ignore undefined types for now (f/F, e/E, g/G, a/A, n)
                }

                if (number) {
                    if (is_signed) {
                        switch (length) {
                            case LENGTH_SHORT_SHORT:
                                print_num_signed((int8_t)va_arg(arg_ptr, int), base);
                                break;
                            case LENGTH_SHORT:
                                print_num_signed((int16_t)va_arg(arg_ptr, int), base);
                                break;
                            case LENGTH_DEFAULT:
                                print_num_signed(va_arg(arg_ptr, int), base);
                                break;
                            case LENGTH_LONG:
                                print_num_signed(va_arg(arg_ptr, long), base);
                                break;
                            case LENGTH_LONG_LONG:
                                print_num_signed(va_arg(arg_ptr, int64_t), base);
                                break;
                        }
                    } else {
                        switch (length) {
                            case LENGTH_SHORT_SHORT:
                                print_num_unsigned((uint8_t)va_arg(arg_ptr, uint32_t), base);
                                break;
                            case LENGTH_SHORT:
                                print_num_unsigned((uint16_t)va_arg(arg_ptr, uint32_t), base);
                                break;
                            case LENGTH_DEFAULT:
                                print_num_unsigned(va_arg(arg_ptr, uint32_t), base);
                                break;
                            case LENGTH_LONG:
                                print_num_unsigned(va_arg(arg_ptr, unsigned long), base);
                                break;
                            case LENGTH_LONG_LONG:
                                print_num_unsigned(va_arg(arg_ptr, uint64_t), base);
                                break;
                        }
                    }
                }

                state = PRINTF_STATE_NORMAL;
                length = LENGTH_DEFAULT;
                base = 10;
                is_signed = false;
                number = false;

                break;
        }

        fmt++;
    }

    va_end(arg_ptr);
}

void print_buffer(const char *p_msg, const void *p_buffer, uint32_t p_length) {
    const uint8_t *byte_buf = (const uint8_t *)p_buffer;

    puts(p_msg);

    for (int i = 0; i < p_length; i++) {
        putc(a_hex_lowercase[byte_buf[i] >> 4]);
        putc(a_hex_lowercase[byte_buf[i] & 0xf]);
    }

    puts("\n");
}