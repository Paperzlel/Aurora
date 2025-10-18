#pragma once

#include <stdbool.h>

bool driver_video_load(void *p_data);
void video_driver_set_hint(int p_hint);

void driver_video_clear();
void driver_video_write_char(char c);