#pragma once

#include <kdefs.h>

typedef struct {
    uint64_t ticks;
    uint32_t time_ms;
    uint8_t time_us;
} timer_t;

/**
 * @brief Initializes the Hardware Abstraction Layer, the part of the kernel that separates the hardware functions from the software implementation.
 * Differs from the CPU architecture in that the hardware available to one PC will be different to that of another and as such must be abstracted
 * differently.
 */
void hal_initialize();

uint64_t hal_get_ticks();

void hal_get_time_relative(timer_t *p_timer);