#include "stdio.h"
#include "x86.h"

void __cdecl start(uint16_t boot_drive)
{
    // Need to do some more things before we can go further
    // Since we're in protected mode, we need to write to the video display memory rather than to the BIOS (goodbye BIOS)
    // First, clear the screen
    clrscr();

    // do_something();
    puts("Booted into bootloader stage 2\n");
    for (int i = 0; i < 30; i++)
    {
        puts("a\n");
    }
    for (;;);
}
