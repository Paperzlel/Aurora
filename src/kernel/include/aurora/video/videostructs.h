#ifndef _AURORA_VIDEOSTRUCTS_H
#define _AURORA_VIDEOSTRUCTS_H

#include <aurora/kdefs.h>

struct Framebuffer
{
    uint8_t *address;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
};

typedef struct VideoDriver {
    const char *name;
    int mode_opt;
    bool (*init)(struct VideoDriver *driver, struct Framebuffer *buffer);
    bool (*fini)();
    void (*clear)(uint8_t, uint8_t, uint8_t);
    void (*set_pixel)(uint16_t, uint16_t, uint8_t, uint8_t, uint8_t);
    void (*write_char)(char);
    
} VideoDriver;

#endif // _AURORA_VIDEOSTRUCTS_H