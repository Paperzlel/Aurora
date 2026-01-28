#ifndef _AURORA_PSF_H
#define _AURORA_PSF_H

#include <stdint.h>

// Magic bytes at the beginning of a PSF1 file. PSF2 has different bytes and will be handled differently
#define PSF1_MAGIC 0x0436

// Flag that is enabled whenever there are 512 characters in the charset for the PSF file. If not, there are 256 characters.
#define PSF1_MODE512    0x01
// Flag if the PSF1 file has a table after the bitmaps. If not, then glyphs are stored in their sequential Unicode order.
#define PSF1_MODEHASTAB 0x02
// Same as `PSF1_MODEHASTAB`. If either are set then a table is present
#define PSF1_MODESEQ    0x04
// Max modes that are defined. Equal to `0101` in binary or having both 512 characters and a Unicode table at the end.
#define PSF1_MAXMODE    0x05

// The separator for different glyphs
#define PSF1_SEPARATOR      0xffff
// The beginning of a sequence of glyphs. Current implementations ignore this.
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
