#include "bochs.h"

#include <arch/arch_frontend.h>

#include <stdio.h>

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

static VESA_Framebuffer bochs_fb;
uint32_t *a_video_mem = NULL;

void bochs_set_register(uint16_t p_index, uint16_t p_value) {
    arch_io_outw(BOCHS_PORT_INDEX, p_index);
    arch_io_outw(BOCHS_PORT_DATA, p_value);
}

bool bochs_initialize(VESA_FramebufferMap *p_info) {
    // Check if info is null, if so, return.
    if (!p_info) {
        return false;
    }

    // Check for support, if available
    arch_io_outw(BOCHS_PORT_INDEX, BOCHS_INDEX_ID);
    uint16_t usable = arch_io_inw(BOCHS_PORT_DATA);
    if (usable < BOCHS_BGA_LATEST) {
        return false;
    }
    printf("Using device BPP %d, width %d, height %d\n", p_info->framebuffer.bpp, p_info->framebuffer.width, p_info->framebuffer.height);

    // for (;;);

    // Available, disable to modify stuff
    bochs_set_register(BOCHS_INDEX_ENABLE, BOCHS_DISABLE);
    // Modify stuff
    bochs_set_register(BOCHS_INDEX_BPP, p_info->framebuffer.bpp);
    bochs_set_register(BOCHS_INDEX_XRES, p_info->framebuffer.width);
    bochs_set_register(BOCHS_INDEX_YRES, p_info->framebuffer.height);
    // Re-enable
    bochs_set_register(BOCHS_INDEX_ENABLE, BOCHS_ENABLE | BOCHS_USE_LFB);

    // TODO: PCI detection, which isn't enabled (yet).
    bochs_fb = p_info->framebuffer;
    a_video_mem = (uint32_t *)p_info->framebuffer.address;

    return true;
}

void bochs_clear() {
    for (int y = 0; y < bochs_fb.height; y++) {
        for (int x = 0; x < bochs_fb.width; x++) {
            a_video_mem[y * (bochs_fb.bytes_per_line / 4) + x] = 0x000000ff;
        }
    }
}

void bochs_write_char(char c) {

}