#pragma once

#include <boot/bootstructs.h>
#include <stdbool.h>

bool bochs_initialize(VESA_FramebufferMap *p_info);

void bochs_clear();
void bochs_write_char(char c);