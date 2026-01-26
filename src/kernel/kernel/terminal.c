#include <aurora/terminal.h>
#include <aurora/video/video.h>
#include <aurora/fs/vfs.h>

#include <aurora/arch/arch.h>
#include <asm/io.h>


bool terminal_initialize()
{
    // Load file
    struct VFS_Handle *f = vfs_open("/dev/font.psf");
    if (!f)
    {
        return false;
    }

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
        // Do the thing where we write a font
        video_set_pixel(400, 400, 255, 0, 0);
        video_set_pixel(399, 400, 255, 0, 0);
        video_set_pixel(398, 400, 255, 0, 0);
    }

    if (arch_is_virtualized())
    {
        outb(0xe9, c);
    }
}
