#include "framebuffer.h"

#include "x86.h"
#include "stdio.h"
#include "memory.h"

typedef struct {
    char vbe_signature[4];          // Should equal "VESA"
    uint16_t vbe_version;           // 0x0300 for VBE 3.0
    uint16_t oem_string_ptr[2];     // ??
    uint8_t capabilities[4];        // ??
    uint16_t video_mode_ptr[2];     // Offset:Segment to the video modes
    uint16_t total_memory;          // # of 64 KiB blocks used
    uint8_t reserved[492];
} __attribute__((packed)) VBE_InfoBlock;

VBE_InfoBlock a_info_block;

void VESA_get_framebuffers(VESA_FramebufferMap *p_out_framebuffers) {
    a_info_block.vbe_signature[0] = 'V';
    a_info_block.vbe_signature[1] = 'E';
    a_info_block.vbe_signature[2] = 'S';
    a_info_block.vbe_signature[3] = 'A';

    uint16_t function_supported = 0;

    x86_VBE_GetVESAInfo(&a_info_block, &function_supported);
    printf("VESA version %x\n", a_info_block.vbe_version);
    printf("VESA segment:offset --> 0x%x:0x%x\n", a_info_block.video_mode_ptr[1], a_info_block.video_mode_ptr[0]);

    uint32_t video_mode_address = segofs_to_linear(a_info_block.video_mode_ptr[1] + &a_info_block, a_info_block.video_mode_ptr[0] + &a_info_block);
    uint16_t *video_mode = &a_info_block + segofs_to_linear(a_info_block.video_mode_ptr[1], a_info_block.video_mode_ptr[0]);

    printf("Linear address of VESA video modes: %x\n", video_mode);
    printf("Value found at the address above: %d\n", video_mode);
}