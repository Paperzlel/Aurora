#pragma once

#include <boot/bootstructs.h>
#include <stdbool.h>

bool vesa_initialize(VESA_FramebufferMap *p_info);

void vesa_clear();
void vesa_write_char(char c);