#include <stdint.h>

#include "memory.h"
#include "stdio.h"

#include <boot/bootstructs.h>

#include <arch/arch_frontend.h>
#include <arch/cpuid/cpuid.h>
#include <drivers/driver_load.h>

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

    CPU_Config cfg;

    if (!cpuid_initialize(&cfg)) {
        printf("Could not initialize CPUID information.");
        goto end;
    }

    printf("CPU features: %s\n", cpuid_get_features());

end:
    while (true) {

    }
}
