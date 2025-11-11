#include <stdint.h>

#include "memory.h"
#include "stdio.h"

#include <boot/bootstructs.h>

#include <arch/arch_frontend.h>
#include <arch/cpuid/cpuid.h>
#include <drivers/driver_load.h>
#include <drivers/video/framebuffer.h>
#include <memory/memory_core.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((cdecl)) cstart(BootInfo *boot)
{
    // Clear BSS data
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    // "Load" VGA driver. We do this first because otherwise any architecture-loading errors will fail silently.
    driver_set_hint(LOAD_TYPE_VIDEO, DRIVER_HINT_USE_VGA);
    driver_load_driver(LOAD_TYPE_VIDEO, NULL);

    // Check CPUID for supported features. Needed here as CPU features may be checked by the CPU architecture
    CPU_Config cfg;
    if (!cpuid_initialize(&cfg)) {
        printf("Could not initialize CPUID information.");
        goto end;
    }

    // Load architecture information (IDT, GDT, ISRs).
    if (!arch_init()) {
        printf("Could not load an architecure backend.\n");
        goto end;
    }
    printf("CPU features: %s\n", cpuid_get_features());

    // Load MemoryRegion info into a valid memory map
    if (!initialize_memory_map(&boot->memory_map)) {
        printf("Failed to initialize memory.\n");
        goto end;
    }

    bool video_driver_loaded = false;
    // Could be running a VM, check hypervisor bit and if so attempt to load bochs
    if (cpuid_supports_feature(CPU_FEATURE_HYPERVISOR, 0) || arch_is_virtualized()) {
        driver_set_hint(LOAD_TYPE_VIDEO, DRIVER_HINT_USE_BOCHS);
        video_driver_loaded = driver_load_driver(LOAD_TYPE_VIDEO, (void *)&boot->framebuffer_map);
    }

    // Load VBE drivers instead. If this fails, we can't use the screen and should attempt to reset.
    if (!video_driver_loaded) {
        driver_set_hint(LOAD_TYPE_VIDEO, DRIVER_HINT_USE_VBE);
        if (!driver_load_driver(LOAD_TYPE_VIDEO, (void *)&boot->framebuffer_map)) {
            printf("Could not load a valid video driver.\n");
            goto end;
        }

        video_driver_loaded = true;
    }

end:
    while (true) {

    }
}
