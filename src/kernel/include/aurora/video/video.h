#ifndef _AURORA_DRIVER_VIDEO_H
#define _AURORA_DRIVER_VIDEO_H

#include <aurora/kdefs.h>

/**
 * @brief Load a video driver implementation. The main implmentations are `bochs`, `VESA` and `vga`, with the first being installed on virtual machines,
 * the second being the primary driver on modern PCs, and the last being the backup text mode driver. To load `vga`, one must pass `NULL` as the struct. All
 * others are loaded by passing the pointer to the VESA structs needed to set the systems up.
 * @param p_data The data for settings up `bochs` and/or `VESA`. Casted to `void *` for simplicity's sake
 * @return `true` on success, and `false` on failure.
 */
bool video_load_driver(void *p_data);

/**
 * @brief Checks to see whether the video driver is running in a text-based mode or a video-based mode. Terminals on text-based modes work differently and hence
 * a difference is required.
 * @return `true` if using a text-based driver (`vga`) and `false` if using a video-based driver (`bochs` or `VESA`).
 */
bool video_is_text_mode();

/**
 * @brief Clears all information on the screen and resets it back to its clear color.
 */
void video_clear_screen();

/**
 * @brief Sets the pixel at a given position to a specific color value. Not supported in text modes, see `terminal_write_char` instead. All colors are stored in
 * RGB8 format, and X/Y positions are relative to the top-left pixel of the screen. 
 * @param x The x position of the pixel
 * @param y The y position of the pixel
 * @param r The red color value to use
 * @param g The green color value to use
 * @param b The blue color value to use
 */
void video_set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);


#endif // _AURORA_DRIVER_VIDEO_H