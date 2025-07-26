#include "stdio.h"
#include "x86.h"

void __cdecl start(uint16_t boot_drive)
{
    puts("Hello world from the kernel!\r\n");
    puts("This is another message because I'm SO AWESOME!!!\r\n");
    return;
}