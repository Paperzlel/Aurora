#include "driver_video.h"

#include <arch/arch_frontend.h>
#include <drivers/driver_load.h>

#include "vga/vga.h"
#include "vesa/vesa_main.h"
#include "bochs/bochs.h"

#include <stdint.h>
#include <boot/bootstructs.h>

typedef struct {
    void (*clear)();
    void (*write_char)(char);

    DriverLoadHint hint;
} VideoDriver;

static VideoDriver a_driver_state;

bool driver_video_load(void *p_data) {
    // No information; load VGA driver

    a_driver_state.clear = vga_clear;
    a_driver_state.write_char = vga_write_char;

    // Clear VGA screen first.
    a_driver_state.clear();

    switch (a_driver_state.hint) {
        // BOCHS adapter
        case DRIVER_HINT_USE_BOCHS: {
            VESA_FramebufferMap *map = (VESA_FramebufferMap *)p_data;
            bool ret = bochs_initialize(map);

            if (!ret) {
                return false;
            }

            a_driver_state.clear = bochs_clear;
            a_driver_state.write_char = bochs_write_char;
        } break;
        // VESA VBE driver (default for real PCs)
        case DRIVER_HINT_USE_VBE: {
            // Cast void back to framebuffer
            VESA_FramebufferMap *buf = (VESA_FramebufferMap *)p_data;
            bool res = vesa_initialize(buf);

            if (!res) {
                return false;
            }
            a_driver_state.clear = vesa_clear;
            a_driver_state.write_char = vesa_write_char;
        } break;
        // Backup driver
        case DRIVER_HINT_USE_VGA:
        default:
            break;
    }
    
    a_driver_state.clear();
    return true;
}

void video_driver_set_hint(int p_hint) {
    a_driver_state.hint = p_hint;
}

void driver_video_clear() {
    //HACK: Loads video state if not set. Only caused if the TSS/GDT/IDT failed to load.
    if (a_driver_state.clear == 0) {
        driver_video_load((void *)0);
    }

    a_driver_state.clear();
}

void driver_video_write_char(char c) {
    if (!a_driver_state.write_char) {
        driver_video_load((void *)0);
    }

    arch_io_outb(0xe9, c);
    a_driver_state.write_char(c);
}