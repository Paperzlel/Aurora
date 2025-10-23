#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <videostructs.h>

bool vga_initialize(VideoDriver *out_driver, Framebuffer *p_buffer);

void vga_clear();
void vga_write_char(char c);
