#pragma once

#include <stdbool.h>

typedef enum {
    LOAD_TYPE_VIDEO,
} DriverLoadType;

bool driver_load_driver(DriverLoadType p_type, void *p_data);