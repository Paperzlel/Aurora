#include <stdint.h>

#include "memory.h"
#include "stdio.h"

#include <boot/bootstructs.h>
#include <cpuid.h>

#include <arch/i686/tasks/test.h>
#include <arch/arch_frontend.h>

#define CPUID_VENDOR_QEMU   "TCGTCGTCGTCG"
#define CPUID_VENDOR_INTEL  "GenuineIntel"

void __attribute__((section(".entry"))) start(BootInfo *boot)
{

    // - Load GDT again
    // - Load IDT
    // - Load ISR
    // - Load driver
    //      - Driver calls to v86 common routines when needed

    // TODO: move to arch_init as a default driver call
    clrscr();
    
    // No GDT, TSS, IDT and so on, hang forever.
    if (!arch_init()) {
        printf("Could not load an architecure backend.\n");     // VGA drivers should be architecture-independent.
        goto end;
    }

    // TODO: Abstract.
    uint8_t *p_test_start = get_task_start();
    uint8_t *p_test_end = get_task_end();
    arch_run_v86_task(p_test_start, p_test_end);

    printf("Hello world from the kernel!!!!\n");

    int ebx, ecx, edx, unused;
    
    int max_cpuid_calls = __get_cpuid_max(0, (void *)0);

    if (max_cpuid_calls == 0) {
        printf("CPUID: CPUID not available for this processor.\n");
    }

    printf("Max CPUID calls: %d\n", max_cpuid_calls);

    // For this function look at CPUID on wikipedia and look at the contents of ECX/EDX for features. May also look at EAX/EBX for other stuff.
    __get_cpuid(1, &unused, &edx, &ebx, &ecx); // Little-endian storing and stuff makes this order have EDX contain the string in full.
    
    printf("Processor ID: %x\n", (unused & 0xf00) >> 8);
    
    if (memcmp((void *)&edx, (void *)CPUID_VENDOR_INTEL, 12) == 0) {
        printf("CPU is an Intel CPU.\n");
    }

    int x = 3 / 0; // Crashes code, for now

end:
    for(;;);
}
