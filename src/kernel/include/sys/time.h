#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include <kernel/kdefs.h>

typedef struct {
    uint64_t ticks;
    uint32_t time_ms;
    uint8_t time_us;
} timer_t;

bool timer_get_time(timer_t *p_timer);

void timer_sleep(uint64_t p_ms);

#endif // _KERNEL_TIMER_H