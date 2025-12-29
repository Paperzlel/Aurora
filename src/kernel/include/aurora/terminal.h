#ifndef _AURORA_TERMINAL_H
#define _AURORA_TERMINAL_H

#include <aurora/kdefs.h>

void terminal_clear_screen();
void terminal_write_char(char c);

void terminal_set_text_colour(uint8_t r, uint8_t g, uint8_t b);
void terminal_clear_text_colour();
void terminal_set_bg_colour(uint8_t r, uint8_t g, uint8_t b);
void terminal_clear_bg_colour();


#endif // _AURORA_TERMINAL_H