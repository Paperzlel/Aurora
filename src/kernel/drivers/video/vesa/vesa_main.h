#pragma once

#include <boot/bootstructs.h>

void vesa_initialize(VESA_FramebufferMap *p_info);

void vesa_clear();
void vesa_write_char(char c);