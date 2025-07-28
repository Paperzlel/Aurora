#include "stdio.h"
#include "memdefs.h"
#include "fat.h"

void __attribute__((cdecl)) start(uint16_t boot_drive) {
    // First, clear the screen
    clrscr();

    puts("Booted into bootloader stage 2.\n");

    DISK out_disk;
    if (!disk_initialize(&out_disk, boot_drive)) {
        printf("Failed to initialize disk controller; unable to launch kernel.\n");
        goto end;
    }

    if (!fat_initialize(&out_disk)) {
        printf("Failed to initialize the FAT file system.\n");
        goto end;
    }

    fat_open(&out_disk, "/dev/NOTES.md");

    printf("Video RAM is at %X.", 0xB8000);
end:
    for (;;);
}
