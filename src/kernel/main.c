#include <stdint.h>
#include "memory.h"

#include "stdio.h"

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((section(".entry"))) start(uint16_t boot_drive)
{
    // Zero the memory between the end of the binary and the start of the uninitialized data (bss contains no value at startup)
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    clrscr();

    printf("Hello world from the kernel!!!!\n");

    return; // Do nothing for mow
}
