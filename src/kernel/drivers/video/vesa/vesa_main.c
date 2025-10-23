#include "vesa_main.h"
#include "vesa_set_mode.h"

#include <stdint.h>
#include <stdio.h>

#include <arch/arch_frontend.h>

static Framebuffer *a_frame_info;
uint32_t *vmem = 0;

uint16_t V_WIDTH = 0;
uint16_t V_HEIGHT = 0;
uint16_t V_DEPTH = 0;
uint16_t V_BPL = 0;
uint16_t V_BPP = 0;


bool vesa_initialize(VideoDriver *out_driver, Framebuffer *p_info) {
    if (!p_info) {
        return false;
    }
    a_frame_info = p_info;

    vmem = (uint32_t *)a_frame_info->address;
    V_WIDTH = a_frame_info->width;
    V_HEIGHT = a_frame_info->height;
    V_DEPTH = a_frame_info->bpp / 8;
    V_BPP = a_frame_info->bpp;

    uint8_t mode = out_driver->mode_opt;

    if (!arch_run_v86_task(&__vesa_start, &__vesa_end, &mode, 1)) {
        printf("Could not enable VESA VBE option %d.\n", mode);
        return false;
    }

    out_driver->clear = vesa_clear;
    out_driver->write_char = vesa_write_char;

    return true;
}

void vesa_clear(uint8_t r, uint8_t g, uint8_t b) {
    for (int x = 0; x < V_WIDTH; x++) {
        for (int y = 0; y < V_HEIGHT; y++) {
            vmem[x + y * V_BPL] = (r << 24) + (g << 16) + (b << 8);
        }
    }
}

void vesa_write_char(char c) {
    vesa_draw_rect(0, 0, 100, 100);
    return;
}

void vesa_draw_rect(int x, int y, int size_x, int size_y) {
    for (int y_ofs = y; y_ofs < size_y; y_ofs++) {
        for (int x_ofs = x; x_ofs < size_x; x_ofs++) {
            vmem[x_ofs + y_ofs * V_BPL] |= 0x00ff0000;
        }
    }
}