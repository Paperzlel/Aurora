#include <stdint.h>
#include <stdio.h>


#define STACK_CHK_GUARD 0x14BD5C4B

uint32_t __stack_chk_guard = STACK_CHK_GUARD;


void __attribute__((noreturn)) __stack_chk_fail(void)
{
#if defined(__is_libk)
    printf("Stack smashing detected. The buffer has been overrun.");
#else

#endif
    while (1) { }
    __builtin_unreachable();
}