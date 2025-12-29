#include <aurora/terminal.h>
#include <aurora/video/video.h>




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
}
