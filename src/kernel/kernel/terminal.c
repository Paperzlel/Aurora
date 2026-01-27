#include <aurora/terminal.h>
#include <aurora/psf.h>
#include <aurora/video/video.h>
#include <aurora/fs/vfs.h>

#include <aurora/arch/arch.h>
#include <asm/io.h>

// NOTE: Include shouldn't really happen here (since I/O is directed to it anyways) but for the sake of text modes
// we'll give an exception.
#define AUR_MODULE "terminal"
#include <aurora/debug.h>

#include <stdlib.h>
#include <string.h>


static void *a_psf_data = NULL;
static uint16_t *a_uc_map = NULL;
static uint8_t a_font_height = 0;

static uint16_t xpos = 0;
static uint16_t ypos = 0;

// PSF fonts are handled in the terminal for now. When loading different files or setting different fonts, move it.

bool psf_initialize(void *p_data, uint32_t p_size)
{
    // Currently handling PSF1 only
    struct PSF1_Header *header = (struct PSF1_Header *)p_data;
    if (!header || header->magic != PSF1_MAGIC)
    {
        LOG_ERROR("Invalid PSF1 header.");
        return false;
    }

    a_font_height = header->char_size;
    a_psf_data = malloc(p_size);
    memcpy(a_psf_data, p_data, p_size);
    // Allocate 65535 bytes for each respective byte. Expensive, but needed.
    // Each glyph is accessed via the Unicode character and its corresponding index.
    a_uc_map = (uint16_t *)malloc(((uint16_t)-1));

    // Now, map the screen font.
    uint16_t *table = (uint16_t *)((uint8_t *)a_psf_data + sizeof(struct PSF1_Header) + 
            (header->char_size) * (header->font_flags & PSF1_MODE512 ? 512 : 256));
    
    uint16_t glyph_idx = 0;
    while (table < (uint16_t *)((uint8_t *)a_psf_data + p_size))
    {
        if (*table == PSF1_SEPARATOR)
        {
            // Next character
            glyph_idx++;
            table++;
        }
        else
        {
            // Add glyph to map
            a_uc_map[*table] = glyph_idx;
            table++;
        }
    }

    // Done mapping data, can go ahead and free file handles
    // Reallocate font data to shave off the table as it's no longer needed
    a_psf_data = realloc(a_psf_data, sizeof(struct PSF1_Header) + a_font_height * 
            (header->font_flags & PSF1_MODE512 ? 512 : 256));
    return true;
}


void *psf_get_char_data(uint16_t p_char)
{
    return a_psf_data + sizeof(struct PSF1_Header) + (a_uc_map[p_char] << 4);
}


bool terminal_initialize()
{
    // Load file
    struct VFS_Handle *f = vfs_open("/dev/font.psf");
    if (!f)
    {
        return false;
    }

    // Read all contents into a buffer
    void *data = vfs_read(f, f->size);
    if (data && !psf_initialize(data, f->size))
    {
        LOG_ERROR("Failed to parse PSF font from \"/dev/font.psf\" .");
        return false;
    }

    vfs_close(f);
    return true;
}

void terminal_clear_screen()
{
    video_clear_screen();
}

void terminal_write_char(char c)
{
    if (video_is_text_mode())
    {
        video_set_pixel(c, 0, 0, 0, 0);
    }
    else
    {
        // Handle characters that aren't valid
        switch (c)
        {
            case '\n':
            {
                xpos = 0;
                ypos += a_font_height + 1;
            } break;
            case '\t':
            {
                xpos += 36 - (xpos % 36);
            } break;
            default:
            {
                // UTF-8 and Unicode share first 255 characters, no issues.
                uint8_t *bits = psf_get_char_data(c);
                // Do the thing where we write a font
                for (int i = 0; i < a_font_height; i++)
                {
                    for (int j = 0; j < 8; j++)
                    {
                        uint8_t fill = (*bits >> (7 - j)) & 1 ? 0xff : 0x00;
                        video_set_pixel(xpos + j, ypos, fill, fill, fill);
                    }

                    ypos++;
                    bits++;
                }

                ypos = 0;
                xpos += 9;
            } break;
        }
    }

    if (arch_is_virtualized())
    {
        outb(0xe9, c);
    }
}
