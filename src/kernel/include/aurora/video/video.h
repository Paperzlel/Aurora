#ifndef _AURORA_DRIVER_VIDEO_H
#define _AURORA_DRIVER_VIDEO_H

#include <aurora/kdefs.h>

bool video_load_driver(void *p_data);
bool video_is_text_mode();

void video_clear_screen();
void video_set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);


#endif // _AURORA_DRIVER_VIDEO_H