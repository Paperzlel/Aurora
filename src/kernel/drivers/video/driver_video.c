#include "driver_video.h"

#include <arch/arch_frontend.h>
#include <drivers/driver_load.h>

#include "vga/vga.h"
#include "vesa/vesa_main.h"
#include "bochs/bochs.h"

#include <stdint.h>
#include <boot/bootstructs.h>
#include <videostructs.h>

VideoDriver a_driver_state;

void vesa_to_driver(VESA_FramebufferMap *p_map, Framebuffer *out_framebuffer) {
    if (!p_map || !out_framebuffer) {
        return;
    }

    if ((p_map->framebuffer.bpp / 8) * p_map->framebuffer.width != p_map->framebuffer.bytes_per_line) {
        return;
    }

    out_framebuffer->address = (uint8_t *)p_map->framebuffer.address;
    out_framebuffer->width = p_map->framebuffer.width;
    out_framebuffer->height = p_map->framebuffer.height;
    out_framebuffer->bpp = p_map->framebuffer.bpp;
}

bool driver_video_load(void *p_data) {
    // No information; load VGA driver

    VESA_FramebufferMap *map = (VESA_FramebufferMap *)p_data;
    Framebuffer fb;
    if (p_data) {
        vesa_to_driver(map, &fb);
    } else {
        // No data, swap to VGA.
        a_driver_state.hint = DRIVER_HINT_USE_VGA;
        // Already loaded VGA drivers
        if (a_driver_state.init != 0) {
            return true;
        }
    }

    switch (a_driver_state.hint) {
        // BOCHS adapter
        case DRIVER_HINT_USE_BOCHS: {
            bool ret = bochs_initialize(&a_driver_state, &fb);

            if (!ret) {
                return false;
            }
        } break;
        // VESA VBE driver (default for real PCs)
        case DRIVER_HINT_USE_VBE: {
            a_driver_state.hint = map->framebuffer.mode_id;
            bool res = vesa_initialize(&a_driver_state, &fb);

            if (!res) {
                return false;
            }
        } break;
        // Backup text-only driver
        case DRIVER_HINT_USE_VGA:
        default: {
            bool res = vga_initialize(&a_driver_state, 0);

            if (!res) {
                return false;
            }
        } break;
    }
    
    a_driver_state.clear(0, 0, 0);

    return true;
}

void video_driver_set_hint(int p_hint) {
    a_driver_state.hint = p_hint;
}

void driver_video_clear() {
    if (a_driver_state.clear == 0) {
        return;
    }

    a_driver_state.clear(0, 0, 0);
}

void driver_video_write_char(char c) {
    if (!a_driver_state.write_char) {
        return;
    }

    if (arch_is_virtualized()) {
        i686_outb(0xe9, c);
    }
    
    a_driver_state.write_char(c);
}