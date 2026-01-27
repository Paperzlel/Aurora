#ifndef _AURORA_PSF_H
#define _AURORA_PSF_H

#include <stdint.h>

#define PSF1_MAGIC 0x0436

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODESEQ    0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR      0xffff
#define PSF1_STARTSEQ       0xfffe


/**
 * @brief Struct for the header of a PSF1 file. PSF1 files always have characters that are 8 bits in width and `char_size` in height.
 * PSF1 and PSF2 act different and hence should be parse differently.
 */
struct PSF1_Header
{
    uint16_t magic;             // Magic value at the beginning of the file to declare it as a PSF file.
    uint8_t font_flags;         // Flags set by the creator to determine how the font should be parsed.
    uint8_t char_size;          // Height of the character in pixels. By default, Aurora uses a 16-bit console font height.
};


#endif // _AURORA_PSF_H
