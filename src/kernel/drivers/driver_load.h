#pragma once

#include <stdbool.h>

typedef enum {
    LOAD_TYPE_VIDEO,
} DriverLoadType;

typedef enum {
    DRIVER_HINT_USE_BOCHS = 0,
    DRIVER_HINT_USE_VBE = 1,
    DRIVER_HINT_USE_VGA = 2,
} DriverLoadHint;

bool driver_load_driver(DriverLoadType p_type, void *p_data);

void driver_set_hint(DriverLoadType p_type, DriverLoadHint p_hint);