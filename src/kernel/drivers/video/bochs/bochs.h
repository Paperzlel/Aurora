#pragma once

#include <videostructs.h>
#include <stdbool.h>

bool bochs_initialize(VideoDriver *out_driver, Framebuffer *p_info);

void bochs_clear();
void bochs_write_char(char c);