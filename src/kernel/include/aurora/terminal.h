#ifndef _AURORA_TERMINAL_H
#define _AURORA_TERMINAL_H

#include <aurora/kdefs.h>

/**
 * @brief Terminal version of clearing the screen. Currently functions the same as `video_clear_screen`. Distinctions may be made in the future.
 */
void terminal_clear_screen();

/**
 * @brief Writes a given character into the terminal. This terminal 
 * @param c The character to write.
 */
void terminal_write_char(char c);

void terminal_set_text_colour(uint8_t r, uint8_t g, uint8_t b);
void terminal_clear_text_colour();
void terminal_set_bg_colour(uint8_t r, uint8_t g, uint8_t b);
void terminal_clear_bg_colour();


#endif // _AURORA_TERMINAL_H