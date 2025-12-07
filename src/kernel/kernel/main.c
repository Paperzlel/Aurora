#include <stdint.h>

#include <string.h>
#include <stdio.h>

#include <boot/bootstructs.h>

#include <kernel/memory.h>
#include <kernel/video/driver_video.h>

#include <kernel/arch/arch.h>
#include <kernel/arch/cpuid.h>
#include <kernel/arch/io.h>
#include <kernel/hal/hal.h>

#define AUR_MODULE "main"
#include <kernel/debug.h>

#include <sys/time.h>

/**
 * @brief Initializes the memory subsystem, including allocators, pre-defined pages, and so on. 
 * @param p_map Pointer to the memory map of the PC. This mainly concerns unmapped memory (such as the BIOS or APIC devices) and also gives us the info
 * as to where free memory is.
 * @param p_kernel_size The size of the kernel in bytes. Since the kernel resides at 0x0010 0000 in physical memory, we can use this to predict where our
 * allocators can start from. The kernel size is static once compiled, so we don't need to worry about this changing.
 * @return `true` if we could initialize the memory, and `false` if something went wrong. Any error here should be treated as failed, and cause an OS panic.
 */
extern bool initialize_memory(MemoryMap *, uint32_t);

extern uint8_t __bss_start;
extern uint8_t __end;

static BootInfo info;

void __attribute__((cdecl)) cstart(BootInfo *boot)
{
    // Clear BSS data
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    memcpy(&info, boot, sizeof(BootInfo));

    // Load architecture information (IDT, GDT, ISRs).
    if (!arch_init()) {
        printf("Could not load an architecure backend.\n");
        goto end;
    }
    
    // "Load" VGA driver. We do this first because otherwise any architecture-loading errors will fail silently.
    if (!driver_video_load(NULL)) {
        panic();    // Lose our minds if this happens, but it should NEVER occur.
    }

    // Check CPUID for supported features. Needed here as CPU features may be checked by the CPU architecture
    CPU_Config cfg;
    if (!cpuid_initialize(&cfg)) {
        printf("Could not initialize CPUID information.\n");
        goto end;
    }

    // Initialize memory info.
    if (!initialize_memory(&boot->memory_map, boot->kernel_size)) {
        printf("Failed to initialize memory.\n");
        goto end;
    }
    hal_initialize(boot->boot_device);
    LOG_INFO("CPU features: %s", cpuid_get_features());

    // Load a basic graphics driver (Bochs VBE, VESA) to draw complex objects in.
    if (!driver_video_load((void *)&boot->framebuffer_map)) {
        LOG_ERROR("Failed to load a non-VGA video driver. Graphics options will not be available.");
        goto end;
    }

end:
    LOG_INFO("%llu ticks since PIT initialized\n", hal_get_ticks());
    for(;;);
}
