#pragma once

typedef enum {
    LOAD_TYPE_VIDEO,
} DriverLoadType;

void driver_load_driver(DriverLoadType p_type, void *p_data);