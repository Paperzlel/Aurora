#pragma once

#include <stdbool.h>
#include <driver_common.h>

bool driver_load_driver(DriverLoadType p_type, void *p_data);

void driver_set_hint(DriverLoadType p_type, DriverLoadHint p_hint);