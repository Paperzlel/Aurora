#include "vesa_main.h"
#include "vesa_set_mode.h"

#include <stdint.h>
#include <stdio.h>

#include <arch/arch_frontend.h>

static VESA_FramebufferMap *a_frame_info;
uint8_t *vmem = 0;

uint16_t V_WIDTH = 0;
uint16_t V_HEIGHT = 0;
uint16_t V_DEPTH = 0;
uint16_t V_BPL = 0;
uint16_t V_BPP = 0;


bool vesa_initialize(VESA_FramebufferMap *p_info) {
    a_frame_info = p_info;

    if ((a_frame_info->framebuffer.bpp / 8) * a_frame_info->framebuffer.width != a_frame_info->framebuffer.bytes_per_line) {
        // Find a way to deal with padding
        vmem = (uint8_t *)a_frame_info->framebuffer.address;
        return false;
    }


    vmem = (uint8_t *)a_frame_info->framebuffer.address;
    V_WIDTH = a_frame_info->framebuffer.width;
    V_HEIGHT = a_frame_info->framebuffer.height;
    V_DEPTH = a_frame_info->framebuffer.bpp / 8;
    V_BPP = a_frame_info->framebuffer.bpp;
    V_BPL = a_frame_info->framebuffer.bytes_per_line;

    uint8_t mode = a_frame_info->framebuffer.mode_id;

    if (!arch_run_v86_task(&__vesa_start, &__vesa_end, &mode, 1)) {
        printf("Could not enable VESA VBE software, an error has occured.");
        return false;
    }
}

void vesa_clear() {
    for (int x = 0; x < V_WIDTH; x++) {
        for (int y = 0; y < V_HEIGHT; y++) {
            vmem[x * V_DEPTH + y * V_BPL] = 128;
            vmem[(x * V_DEPTH + y * V_BPL) + 1] = 128;
            vmem[(x * V_DEPTH + y * V_BPL) + 2] = 128;
            vmem[(x * V_DEPTH + y * V_BPL) + 3] = 0xf1;
        }
    }
}

void vesa_write_char(char c) {
    return;
}
