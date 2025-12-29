#include <aurora/video/video.h>

#include <aurora/arch/cpuid.h>
#include <aurora/arch/arch.h>
#include <asm/io.h>

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


bool video_load_driver(void *p_data)
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

        // Use bochs driver on VMs if supported. Fallback to VESA if not virtualized or if bochs failed.
        if (cpuid_supports_feature(CPU_FEATURE_HYPERVISOR, 0) || arch_is_virtualized())
        {
            a_driver_state = a_bochs_driver;
        }
        else
        {
            a_driver_state = a_vesa_driver;
        }
        
        a_driver_state.mode_opt = map->framebuffer.mode_id;
    }

    bool success = a_driver_state.init(&a_driver_state, &fb);

    if (!success)
    {
        if (!p_data)
        {
            return false;
        }

        LOG_ERROR("Failed to load video driver %s, falling back to VGA support...", a_driver_state.name);
        return video_load_driver((void *)0);
    }
    
    // Clear screen once done.
    a_driver_state.clear(0, 0, 0);
    return true;
}


bool video_is_text_mode()
{
    return a_driver_state.mode_opt > 0 ? false : true;
}


void video_clear_screen()
{
    if (a_driver_state.clear == 0)
    {
        return;
    }

    a_driver_state.clear(0, 0, 0);
}


void video_set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
    if (a_driver_state.mode_opt == -1)
    {
        a_driver_state.write_char(x);
        return;
    }

    a_driver_state.set_pixel(x, y, r, g, b);
}
