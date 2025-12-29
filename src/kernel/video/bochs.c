#include <asm/io.h>

#define __need_NULL
#include <stddef.h>

#include <aurora/video/framebuffer.h>

#define BOCHS_PORT_INDEX 0x01ce
#define BOCHS_PORT_DATA 0x01cf

#define BOCHS_INDEX_ID 0 
#define BOCHS_INDEX_XRES 1
#define BOCHS_INDEX_YRES 2
#define BOCHS_INDEX_BPP 3
#define BOCHS_INDEX_ENABLE 4

#define BOCHS_BGA_LATEST 0xb0c5

#define BOCHS_USE_LFB 0x40
#define BOCHS_ENABLE 0x01
#define BOCHS_DISABLE 0x00

void bochs_set_register(uint16_t p_index, uint16_t p_value)
{
    outw(BOCHS_PORT_INDEX, p_index);
    outw(BOCHS_PORT_DATA, p_value);
}

bool bochs_initialize(struct VideoDriver *out_driver, struct Framebuffer *p_info)
{
    // Check if info is null, if so, return.
    if (!p_info)
    {
        return false;
    }

    // Check for support, if available
    outw(BOCHS_PORT_INDEX, BOCHS_INDEX_ID);
    uint16_t usable = inw(BOCHS_PORT_DATA);
    if (usable < BOCHS_BGA_LATEST)
    {
        return false;
    }

    // Available, disable to modify stuff
    bochs_set_register(BOCHS_INDEX_ENABLE, BOCHS_DISABLE);
    // Modify stuff
    bochs_set_register(BOCHS_INDEX_BPP, p_info->bpp);
    bochs_set_register(BOCHS_INDEX_XRES, p_info->width);
    bochs_set_register(BOCHS_INDEX_YRES, p_info->height);
    // Re-enable
    bochs_set_register(BOCHS_INDEX_ENABLE, BOCHS_ENABLE | BOCHS_USE_LFB);

    // TODO: PCI detection, which isn't enabled (yet).
    // We shouldn't need this now, since the VESA framebuffer already does this, but we may need to in the future should that get abstracted.

    framebuffer_intialize(out_driver, p_info);

    return true;
}

struct VideoDriver a_bochs_driver =
{
    "bochs",
    -1,
    bochs_initialize,
    NULL,
    framebuffer_clear,
    framebuffer_set_pixel,
    framebuffer_write_char,
};
