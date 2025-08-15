#include <stdint.h>

#include "memory.h"
#include "stdio.h"

#include <boot/bootstructs.h>
#include <cpuid.h>

extern uint8_t __bss_start;
extern uint8_t __end;

#define CPUID_VENDOR_QEMU   "TCGTCGTCGTCG"
#define CPUID_VENDOR_INTEL  "GenuineIntel"

void __attribute__((section(".entry"))) start(BootInfo *boot)
{
    // Zero the memory between the end of the binary and the start of the uninitialized data (bss contains no value at startup)
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    clrscr();

    printf("Hello world from the kernel!!!!\n");

    int ebx, ecx, edx, unused;
    
    int max_cpuid_calls = __get_cpuid_max(0, (void *)0);

    if (max_cpuid_calls == 0) {
        printf("CPUID: CPUID not available for this processor.\n");
    }

    printf("Max CPUID calls: %d\n", max_cpuid_calls);

    // For this function look at CPUID on wikipedia and look at the contents of ECX/EDX for features. May also look at EAX/EBX for other stuff.
    __get_cpuid(0, &unused, &edx, &ebx, &ecx); // Little-endian storing and stuff makes this order have EDX contain the string in full.
    
    printf("EAX after 0 is called: %d\n", unused);
    
    if (memcmp((void *)&edx, (void *)CPUID_VENDOR_INTEL, 12) == 0) {
        printf("CPU is an Intel CPU.\n");
    }

    return; // Do nothing for mow
}
