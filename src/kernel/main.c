#include <stdint.h>

#include "memory.h"
#include "stdio.h"

#include <boot/bootstructs.h>
#include <cpuid.h>

#include "i686/gdt.h"
#include "i686/idt.h"
#include "i686/isr.h"

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
    //      - Driver may register a handle for any GPF that could occur

    // v86 looks something like this
    // entering:
    // params: ss, esp, cs, eip
    // mov ebp, esp
    // push dword [ebp + 4]
    // push dword [ebp + 8]
    // pushfd
    // or dword [esp], (1 << 17)
    // push dword [ebp + 12]
    // push dword [ebp + 16]
    // iret                         ; IRET acts as if the data above is an interrupt, and is "tricking" our code into entering V86

    // exiting is easier, have a v86 interrupt (see https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-system-programming-manual-325384.pdf)
    // See 20.3.3.4 of above for how to deal with calling software interrupts

    clrscr();

    i686_gdt_initialize();
    i686_isr_initialize();
    i686_idt_initialize();

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
