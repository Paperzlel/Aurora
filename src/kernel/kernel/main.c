#include <stdint.h>

#include <string.h>
#include <stdio.h>

#include <boot/bootstructs.h>

#include <aurora/memory.h>
#include <aurora/video/video.h>

#include <aurora/arch/arch.h>
#include <aurora/arch/cpuid.h>
#include <asm/io.h>
#include <aurora/hal/hal.h>

#include <aurora/fs/vfs.h>

#define AUR_MODULE "main"
#include <aurora/debug.h>

#include <sys/time.h>

extern uint8_t __bss_start;
extern uint8_t __end;

extern bool terminal_initialize();

static struct BootInfo info;

void __attribute__((cdecl)) cstart(struct BootInfo *boot)
{
    // Clear BSS data
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    memcpy(&info, boot, sizeof(struct BootInfo));

    // Load architecture information (IDT, GDT, ISRs).
    if (!arch_init())
    {
        printf("Could not load an architecure backend.\n");
        goto end;
    }
    
    // "Load" VGA driver. We do this first because otherwise any architecture-loading errors will fail silently.
    if (!video_load_driver(NULL))
    {
        panic();    // Lose our minds if this happens, but it should NEVER occur.
    }

    // Check CPUID for supported features. Needed here as CPU features may be checked by the CPU architecture
    struct CPU_Config cfg;
    if (!cpuid_initialize(&cfg))
    {
        printf("Could not initialize CPUID information.\n");
        goto end;
    }

    // Initialize memory info.
    if (!initialize_memory(&boot->memory_map, boot->kernel_size))
    {
        printf("Failed to initialize memory.\n");
        goto end;
    }
    hal_initialize(boot->boot_device);
    LOG_INFO("CPU features: %s", cpuid_get_features());

    // Init VFS so we can load some font resources
    if (!vfs_initialize())
    {
        LOG_ERROR("Failed to initialize VFS.");
        goto end;
    }

    // Setup terminal
    if (!terminal_initialize())
    {
        LOG_ERROR("Failed to initialize terminal.");
        goto end;
    }

    // Load a basic graphics driver (Bochs VBE, VESA) to draw complex objects in.
    if (!video_load_driver((void *)&boot->framebuffer_map))
    {
        LOG_ERROR("Failed to load a non-VGA video driver. Graphics options will not be available.");
        goto end;
    }

end:
    LOG_DEBUG("Here's a fancy message\n\tthat appears on the screen!");
    for(;;);
}
