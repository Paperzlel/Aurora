#ifndef _AURORA_VIDEOSTRUCTS_H
#define _AURORA_VIDEOSTRUCTS_H

#include <aurora/kdefs.h>

// Structure representing the framebuffer information used by our driver implementations.
struct Framebuffer
{
    uint8_t *address;       // Pointer to the framebuffer, as physical memory.
    uint16_t width;         // The width of the framebuffer in pixels
    uint16_t height;        // The height of the framebuffer in pixels
    uint8_t bpp;            // The number of bits stored per pixel, usually 24 or 32
};

// Structure representing all common functions between a video driver.
struct VideoDriver
{
    // The name of the device being used
    const char *name;
    // The "video mode" that is set by the driver
    int mode_opt;
    // The initialization function
    bool (*init)(struct VideoDriver *driver, struct Framebuffer *buffer);
    // The finalization function
    bool (*fini)();
    // The clearing screen function
    void (*clear)(uint8_t, uint8_t, uint8_t);
    // The setting pixel function
    void (*set_pixel)(uint16_t, uint16_t, uint8_t, uint8_t, uint8_t);
    // The write char function
    void (*write_char)(char);
    
};

#endif // _AURORA_VIDEOSTRUCTS_H