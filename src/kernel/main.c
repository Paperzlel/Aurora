#include <stdint.h>

#include "memory.h"
#include "stdio.h"

#include <boot/bootstructs.h>
#include <cpuid.h>

#include "i686/gdt.h"
#include "i686/idt.h"
#include "i686/isr.h"
#include "i686/tss.h"
#include "i686/tasks/test.h"
#include "i686/v86/v86_monitor.h"

extern uint8_t __bss_start;
extern uint8_t __end;

#define CPUID_VENDOR_QEMU   "TCGTCGTCGTCG"
#define CPUID_VENDOR_INTEL  "GenuineIntel"

void __attribute__((section(".entry"))) start(BootInfo *boot)
{

    // - Load GDT again
    // - Load IDT
    // - Load ISR
    // - Load driver
    //      - Driver calls to v86 common routines when needed

    clrscr();

    i686_gdt_initialize();
    i686_tss_initialize();
    i686_isr_initialize();
    i686_idt_initialize();

    // TODO: Abstract.
    uint8_t *p_test_start = get_task_start();
    uint8_t *p_test_end = get_task_end();
    v86_load_task(p_test_start, p_test_end);

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

    int x = 3 / 0; // Crashes code, for now

end:
    for(;;);
}
