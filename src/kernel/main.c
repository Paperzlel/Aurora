#include <stdint.h>

#include "memory.h"
#include "stdio.h"

#include <boot/bootstructs.h>
#include <cpuid.h>

#include <arch/arch_frontend.h>
#include <drivers/driver_load.h>

//TODO: Remove.
#include <drivers/video/bochs/bochs.h>

#define CPUID_VENDOR_QEMU   "TCGTCGTCGTCG"
#define CPUID_VENDOR_INTEL  "GenuineIntel"

void __attribute__((section(".entry"))) start(BootInfo *boot)
{
    // No GDT, TSS, IDT and so on, hang forever.
    if (!arch_init()) {
        printf("Could not load an architecure backend.\n");     // VGA drivers should be architecture-independent.
        goto end;
    }

    // Do CPUID stuff first - get CPU information (model and features)
    // Once done, set up paging and other information.
    // Then, run video driver stuff (find if bochs VGA drivers are used, then run from there).
    // We can get the rest of the kernel from there.

    int ebx, ecx, edx, eax;
    int max_cpuid_calls = __get_cpuid_max(0, (void *)0);

    if (max_cpuid_calls == 0) {
        printf("CPUID: CPUID not available for this processor.\n");
    }

    printf("Max CPUID calls: %d\n", max_cpuid_calls);

    // For this function look at CPUID on wikipedia and look at the contents of ECX/EDX for features. May also look at EAX/EBX for other stuff.
    __get_cpuid(0, &eax, &ebx, &ecx, &edx); // Little-endian storing and stuff makes this order have EDX contain the string in full.
    
    char name[13] = {0};
    memcpy(name, (void *)&ebx, 4);
    memcpy(name + 4, (void *)&edx, 4);
    memcpy(name + 8, (void *)&ecx, 4);
    printf("CPU name is %s\n", name);
    
    if (memcmp(name, (void *)CPUID_VENDOR_INTEL, 12) == 0) {
        printf("CPU is an Intel CPU.\n");
    }

end:
    while (true) {

    }
}
