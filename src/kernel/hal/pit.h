#pragma once

#include <kdefs.h>

void pit_initialize();

uint64_t pit_get_ticks();

uint32_t pit_get_frequency();