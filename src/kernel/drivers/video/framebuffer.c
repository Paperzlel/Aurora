#include "framebuffer.h"

#include <memory/paging.h>

static Framebuffer data;
static PageTableHandle fb_page_handle;

bool framebuffer_intialize(VideoDriver *out_driver, Framebuffer *p_info) {
    if (!p_info || !p_info->address || data.address != 0) {
        return false;
    }

    data.address = p_info->address;
    data.width = p_info->width;
    data.height = p_info->height;
    data.bpp = p_info->bpp;

    // Page allocate the FB to address 0xb0000000
    if (!paging_map_region((void *)data.address, (void *)0xb0000000, data.width * data.height * (data.bpp / 8), &fb_page_handle)) {
        return false;
    }
    // Re-assign if successful.
    data.address = (uint8_t *)0xb0000000;

    // Write test colour to screen
    data.address[0] = 12;
    if (data.address[0] != 12) {
        return false;
    }

    out_driver->clear = framebuffer_clear;
    out_driver->set_pixel = framebuffer_set_pixel;
    out_driver->write_char = framebuffer_write_char;

    return true;
}

void framebuffer_clear(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t bytes_per_pixel = data.bpp / 8;

    for (int y = 0; y < data.height; y++) {
        for (int x = 0; x < data.width; x++) {
            int pixel = (x + y * data.width) * bytes_per_pixel;
            data.address[pixel] = b;
            data.address[pixel + 1] = g;
            data.address[pixel + 2] = r;
        }
    }
}

void framebuffer_set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    data.address[x * (data.bpp / 8) + y * data.width + 1] = r;
    data.address[x * (data.bpp / 8) + y * data.width + 2] = g;
    data.address[x * (data.bpp / 8) + y * data.width + 3] = b;
}

void framebuffer_write_char(char c) {

}