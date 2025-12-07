#include <aurora/video/driver_video.h>
#include <aurora/console.h>

#include <aurora/arch/cpuid.h>
#include <aurora/arch/arch.h>
#include <asm/io.h>

#include "vga/vga.h"
#include "vesa/vesa_main.h"
#include "bochs/bochs.h"

#include <stdint.h>
#include <boot/bootstructs.h>
#include <aurora/video/videostructs.h>

#define AUR_MODULE "video"
#include <aurora/debug.h>

extern struct VideoDriver a_vga_driver;
extern struct VideoDriver a_vesa_driver;
extern struct VideoDriver a_bochs_driver;

static VideoDriver a_driver_state;

static void vesa_to_internal(struct VESA_FramebufferInfo *p_map, struct Framebuffer *out_framebuffer)
{
    if (!p_map || !out_framebuffer)
    {
        return;
    }

    if ((p_map->framebuffer.bpp / 8) * p_map->framebuffer.width != p_map->framebuffer.bytes_per_line)
    {
        return;
    }

    out_framebuffer->address = (uint8_t *)p_map->framebuffer.address;
    out_framebuffer->width = p_map->framebuffer.width;
    out_framebuffer->height = p_map->framebuffer.height;
    out_framebuffer->bpp = p_map->framebuffer.bpp;
}

bool driver_video_load(void *p_data)
{
    struct Framebuffer fb;
    if (!p_data)
    {
        a_driver_state = a_vga_driver;
    }
    else
    {
        struct VESA_FramebufferInfo *map = (struct VESA_FramebufferInfo *)p_data;
        vesa_to_internal(map, &fb);
        // Failed to map, throw error
        // VGA _SHOULD_ be loaded here
        if (!fb.address)
        {
            LOG_ERROR("Failed to load VESA information.");
            LOG_ERROR("Card: %s", map->product_name);
            LOG_ERROR("Vendor: %s", map->vendor_name);
            return false;
        }
        a_driver_state.mode_opt = map->framebuffer.mode_id;

        // Use bochs driver on VMs if supported. Fallback to VESA if not virtualized or if bochs failed.
        if (cpuid_supports_feature(CPU_FEATURE_HYPERVISOR, 0) || arch_is_virtualized())
        {
            a_driver_state = a_bochs_driver;
        }
        else
        {
            a_driver_state = a_vesa_driver;
        }
    }

    bool success = a_driver_state.init(&a_driver_state, &fb);

    if (!success)
    {
        if (!p_data)
        {
            return false;
        }

        LOG_ERROR("Failed to load video driver %s, falling back to VGA support...", a_driver_state.name);
        return driver_video_load((void *)0);
    }
    
    // Clear screen once done.
    a_driver_state.clear(0, 0, 0);
    return true;
}

void driver_video_clear()
{
    if (a_driver_state.clear == 0)
    {
        return;
    }

    a_driver_state.clear(0, 0, 0);
}

void driver_video_write_char(char c)
{
    if (arch_is_virtualized())
    {
        outb(0xe9, c);
    }
    
    if (!a_driver_state.write_char)
    {
        // PANIC, write exclamation marks to the screen.
        for (int i = 0; i < 24; i++)
        {
            vga_write_char('!');
        }
        return;
    }
    
    a_driver_state.write_char(c);
}