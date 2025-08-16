#include "stdio.h"
#include "memdefs.h"
#include "fat.h"
#include "memory.h"
#include "memdetect.h"
#include "framebuffer.h"

#include <boot/bootstructs.h>

uint8_t *kernel =           (uint8_t *)KERNEL_BASE_ADDR;
uint8_t *kernel_load_buf =  (uint8_t *)KERNEL_LOAD_ADDR;

typedef void (*kmain)(BootInfo *);

void __attribute__((cdecl)) start(uint16_t boot_drive) {
    // First, clear the screen
    clrscr();

    puts("Booted into bootloader stage 2.\n");

    BootInfo boot;
    boot.boot_device = boot_drive;
    memory_get_mem_map(&boot.memory_map);

    // There is a small chance we COULD run the OS without any framebuffer information, but since we need it for everything else we're going to throw
    // an error here instead.
    if (!VESA_get_framebuffers(&boot.framebuffer_map)) {
        printf("Failed to obtain VESA framebuffers, unable to draw.\n");
        goto end;
    }

    VESA_Framebuffer fb = boot.framebuffer_map.framebuffer;
    printf("Using framebuffer with resolution %dx%d and BPP %d.\n", fb.width, fb.height, fb.bpp);

    DISK out_disk;
    if (!disk_initialize(&out_disk, boot_drive)) {
        printf("Failed to initialize disk controller; unable to launch kernel.\n");
        goto end;
    }

    if (!fat_initialize(&out_disk)) {
        printf("Failed to initialize the FAT file system.\n");
        goto end;
    }

    FAT_File *file = fat_open(&out_disk, "kernel.bin");
    uint32_t read;
    uint8_t *buf = kernel;

    printf("%d bytes in file.\n", file->size);

    while((read = fat_read(&out_disk, file, KERNEL_LOAD_SIZE, kernel_load_buf))) {
        memcpy(buf, kernel_load_buf, read);
        buf += read;
    }
    fat_close(file);

    kmain kernel_start = (kmain)kernel;
    // kernel_start(&boot);
    
end:
    for (;;);
}
