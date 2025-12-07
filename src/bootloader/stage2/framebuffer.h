#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <boot/bootstructs.h>

/**
 * @brief Obtains a framebuffer from the list of available VESA VBE framebuffers. Currently, the bootloader obtains the map 800x600x32, however higher resolutions
 * will be available via GPU drivers, if the OS reaches that point.
 * @param p_out_framebuffer The output framebuffer map to fill in
 * @returns True if successful, and false if an error occurs, mainly due to a lower version or an inability to obtain the function data.
 */
bool VESA_get_framebuffer(struct VESA_FramebufferInfo *p_out_framebuffer);
