#pragma once

#include <aurora/kdefs.h>
#include <aurora/video/videostructs.h>

bool vga_initialize(struct VideoDriver *out_driver, struct Framebuffer *p_buffer);

void vga_clear(uint8_t r, uint8_t g, uint8_t b);
void vga_write_char(char c);
