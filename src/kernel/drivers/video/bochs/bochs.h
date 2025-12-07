#pragma once

#include <aurora/video/videostructs.h>
#include <stdbool.h>

bool bochs_initialize(struct VideoDriver *out_driver, struct Framebuffer *p_info);

void bochs_clear();
void bochs_write_char(char c);
