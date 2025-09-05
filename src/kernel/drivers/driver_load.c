#include "driver_load.h"

#include "video/driver_video.h"

bool driver_load_driver(DriverLoadType p_type, void *p_data) {
    switch (p_type) {
        case LOAD_TYPE_VIDEO: {
            return driver_video_load(p_data);
        } break;
        default:
            return false;
    }
}