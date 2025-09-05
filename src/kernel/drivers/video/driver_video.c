#include "driver_video.h"

#include <arch/arch_frontend.h>

#include "vga/vga.h"
#include "vesa/vesa_main.h"

#include <stdint.h>
#include <boot/bootstructs.h>

typedef struct {
    void (*clear)();
    void (*write_char)(char);
} VideoDriver;

static VideoDriver a_driver_state;

bool driver_video_load(void *p_data) {
    // No information; load VGA driver

    a_driver_state.clear = vga_clear;
    a_driver_state.write_char = vga_write_char;

    if (p_data) {
        a_driver_state.clear();
        // Cast void back to framebuffer
        VESA_FramebufferMap *buf = (VESA_FramebufferMap *)p_data;
        vesa_initialize(buf);

        a_driver_state.clear = vesa_clear;
        a_driver_state.write_char = vesa_write_char;
    }
    
    // a_driver_state.clear();
    return true;
}

void driver_video_clear() {
    //HACK: Loads video state if not set. Only caused if the TSS/GDT/IDT failed to load.
    if (a_driver_state.clear == 0) {
        driver_video_load(0);
    }

    a_driver_state.clear();
}

void driver_video_write_char(char c) {
    arch_io_outb(0xe9, c);
    a_driver_state.write_char(c);
}