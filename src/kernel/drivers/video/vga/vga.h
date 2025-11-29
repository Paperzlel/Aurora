#pragma once

#include <kernel/kdefs.h>
#include <videostructs.h>

bool vga_initialize(VideoDriver *out_driver, Framebuffer *p_buffer);

void vga_clear(uint8_t r, uint8_t g, uint8_t b);
void vga_write_char(char c);
