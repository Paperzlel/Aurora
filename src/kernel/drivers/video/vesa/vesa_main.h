#pragma once

#include <aurora/video/videostructs.h>
#include <stdbool.h>

bool vesa_initialize(struct VideoDriver *out_driver, struct Framebuffer *p_info);

void vesa_clear(uint8_t r, uint8_t g, uint8_t b);
void vesa_write_char(char c);
void vesa_draw_rect(int x, int y, int size_x, int size_y);
