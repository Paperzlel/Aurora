#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <aurora/video/videostructs.h>

bool framebuffer_intialize(struct VideoDriver *out_driver, struct Framebuffer *p_info);

void framebuffer_clear(uint8_t r, uint8_t g, uint8_t b);
void framebuffer_set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);

void framebuffer_write_char(char c);
