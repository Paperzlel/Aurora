#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver_common.h"

typedef struct {
    uint8_t *address;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
} Framebuffer;

typedef struct VideoDriver {
    bool (*init)(struct VideoDriver *driver, Framebuffer *buffer);
    bool (*fini)();
    void (*clear)(uint8_t, uint8_t, uint8_t);
    void (*set_pixel)(uint16_t, uint16_t, uint8_t, uint8_t, uint8_t);
    void (*write_char)(char);

    DriverLoadHint hint;
    int mode_opt;
} VideoDriver;

