#include "stdio.h"
#include "memdefs.h"
#include "fat.h"
#include "elf.h"
#include "memory.h"
#include "memdetect.h"
#include "framebuffer.h"

#include <boot/bootstructs.h>
// Pointer to where the kernel is loaded into memory (a different address since the kernel may be larger than expected)
static uint8_t *kernel_load_buf =  (uint8_t *)KERNEL_LOAD_ADDR;

// Function pointer to the "main" function for our kernel
typedef void (*kmain)(struct BootInfo *);

static struct BootInfo boot = { 0 };

/**
 * @brief Start function for the C part of the bootloader. Obtains basic information about the system, then loads the kernel and passes control over to it.
 * @param boot_drive The drive that the bootloader was selected from
 */
void __attribute__((cdecl)) start(uint16_t boot_drive)
{
    // First, clear the screen
    clrscr();

    boot.boot_device = boot_drive;
    memory_get_mem_map(&boot.memory_map);

    bool can_map_kernel = false;
    // Confirm if the kernel can be loaded at its address
    for (int i = 0; i < boot.memory_map.region_count; i++)
    {
        struct MemoryRegion r = boot.memory_map.regions[i];

        // Check if the memory region is usable
        if (!(r.extended_attribs & MEMORY_REGION_USABLE)) {
            continue;
        }

        // Can map directly to a region
        if (r.base_address == (uint32_t)KERNEL_BASE_ADDR) {
            can_map_kernel = true;
            break;
        }

        // Is not the bottom of a region, but the kernel can map to a part of it
        if (r.base_address < (uint32_t)KERNEL_BASE_ADDR && r.base_address + r.length > (uint32_t)KERNEL_BASE_ADDR) {
            can_map_kernel = true;
            break;
        }
    }

    // Kernel doesn't map to the given address, panic
    // Some form of re-addressing should happen, but it's 2025 and most PCs will have more than 1 MiB of RAM to use so this should never happen in the
    // real world.
    if (!can_map_kernel)
    {
        printf("Could not load the kernel to address 0x%x as there was no memory available.\n", KERNEL_BASE_ADDR);
        goto end;
    }
    
    // There is a small chance we COULD run the OS without any framebuffer information, but since we need it for everything else we're going to throw
    // an error here instead.
    if (!VESA_get_framebuffer(&boot.framebuffer_map))
    {
        printf("Failed to obtain VESA framebuffers, unable to draw.\n");
        goto end;
    }

    struct DISK out_disk;
    if (!disk_initialize(&out_disk, boot_drive))
    {
        printf("Failed to initialize disk controller; unable to launch kernel.\n");
        goto end;
    }

    if (!fat_initialize(&out_disk))
    {
        printf("Failed to initialize the FAT file system.\n");
        goto end;
    }

    struct FAT_File *file = fat_open(&out_disk, "kernel.elf");
    // Load kernel into buffer
    uint32_t read = fat_read(&out_disk, file, KERNEL_LOAD_SIZE, kernel_load_buf);
    fat_close(file);
    
    if (!read)
    {
        printf("Stage 2: Failed to read the FAT data into memory.\n");
        goto end;
    }
    printf("Loading %d bytes from the kernel...\n", read);
    
    kmain kernel_start = NULL;
    if (elf_is_elf(kernel_load_buf))
    {
        // Load rest of ELF header; it's segmented so it needs to be loaded differently

        printf("Stage 2: Checking ELF data is the correct format.\n");
        if (!elf_is_valid_format((struct ELF_Header *)kernel_load_buf))
        {
            printf("Invalid format for ELF file.\n");
            goto end;
        }

        void *entrypoint = NULL;

        printf("Stage 2: Loading ELF file data into memory...\n");
        if (!elf_read((struct ELF_Header *)kernel_load_buf, kernel_load_buf, &read, &entrypoint))
        {
            printf("Unable to read ELF file.\n");
            goto end;
        }

        boot.kernel_size = read;
        printf("Stage 2: Loaded ELF successfully and found kernel entrypoint at %x.\n", entrypoint);
        kernel_start = (kmain)entrypoint;
    }
    else
    {
        printf("Stage 2: Could not detect ELF, loading raw bin...\n");
        // Assumes that the file is a .BIN and can be read directly
        // This mode will probably be deprecated in the future
        uint8_t *kernel = (uint8_t *)KERNEL_BASE_ADDR;
        memcpy(kernel, kernel_load_buf, read);
        boot.kernel_size = read;

        kernel_start = (kmain)kernel;
    }

    printf("Stage 2: Entering kernel...\n");
    kernel_start(&boot);
    
end:
    for (;;);
}
