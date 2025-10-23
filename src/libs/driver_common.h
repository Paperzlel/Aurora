#pragma once

typedef enum {
    LOAD_TYPE_VIDEO,
} DriverLoadType;

typedef enum {
    DRIVER_HINT_USE_BOCHS = 0,
    DRIVER_HINT_USE_VBE = 1,
    DRIVER_HINT_USE_VGA = 2,
} DriverLoadHint;