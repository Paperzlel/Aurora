#include <stdint.h>

// #include "memory.h"
#include "stdio.h"

#include <boot/bootstructs.h>

#include <arch/arch_frontend.h>
#include <arch/cpuid/cpuid.h>
#include <drivers/driver_load.h>
#include <memory/paging.h>

void cstart(BootInfo *boot)
{
    // // Initialize paging
    // if (!paging_initialize(boot->kernel_size)) {
    //     printf("Could not initialize paging.\n");
    //     goto end;
    // }

    // "Load" VGA driver. We do this first because otherwise any architecture-loading errors will fail silently.
    driver_set_hint(LOAD_TYPE_VIDEO, DRIVER_HINT_USE_VGA);
    driver_load_driver(LOAD_TYPE_VIDEO, NULL);

    // Load architecture information (IDT, GDT, ISRs).
    if (!arch_init()) {
        printf("Could not load an architecure backend.\n");
        goto end;
    }

    // Check CPUID for supported features.
    CPU_Config cfg;
    if (!cpuid_initialize(&cfg)) {
        printf("Could not initialize CPUID information.");
        goto end;
    }

    printf("CPU features: %s\n", cpuid_get_features());

    bool video_driver_loaded = false;
    // Could be running a VM, check hypervisor bit and if so attempt to load bochs
    if (cpuid_supports_feature(CPU_FEATURE_HYPERVISOR, 0)) {
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

    int *p_check_in_unmapped = (int *)0xb0000000;
    *p_check_in_unmapped = 33;
    

end:
    while (true) {

    }
}
