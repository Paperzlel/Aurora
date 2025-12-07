#ifndef _AURORA_DRIVER_VIDEO_H
#define _AURORA_DRIVER_VIDEO_H

#include <stdbool.h>

bool driver_video_load(void *p_data);

void driver_video_clear();
void driver_video_write_char(char c);

#endif // _AURORA_DRIVER_VIDEO_H