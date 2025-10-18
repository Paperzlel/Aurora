#include "framebuffer.h"

#include "x86.h"
#include "stdio.h"
#include "memory.h"
#include "string.h"

typedef struct {
    char vbe_signature[4];          // Should equal "VESA"
    uint16_t vbe_version;           // 0x0300 for VBE 3.0
    uint16_t oem_string_ptr[2];     // Pointer to the OEM name
    uint8_t capabilities[4];        // Capabilities flag (Use first 8 bits, other 24 are reserved)
    uint16_t video_mode_ptr[2];     // Offset:Segment to the video modes
    uint16_t total_memory;          // # of 64 KiB blocks used

    // VBE 2.0

    uint8_t oem_software_version[2];            // High byte = major, low byte = minor
    uint16_t vendor_name_ptr[2];                // Segment:Offset to vendor name (little-endian)
    uint16_t product_name_ptr[2];               // Segment:Offset to product name (little-endian)
    uint16_t product_revision_str_ptr[2];       // Segment:Offset to revision string (i.e. the VGA BIOS distribution date) (little-endian)
    uint16_t vbe_af_version;                    // If capabilities bit 3 set, VBE/AF version (0x100 for v1.0P)
    uint16_t accelerated_video_modes_ptr[2];    // If capabilities bit 3 set, pointer to list of supported accelerated video modes
    uint8_t reserved[472];
} __attribute__((packed)) VBE_InfoBlock;

// In the future, if we try to support full 1080p drivers we should really forward this info to the kernel, but since that's a lot of work we won't do that.
// Do bear this in mind later on however, as it may well be worth it.

// To find more details, open INTERRUP.A and search V-104F01

typedef struct {
    uint16_t attributes;                            // Video mode attributes (bitmask)
    uint8_t window_attribs_a;                       // Window attributes (deprecated)
    uint8_t window_attribs_b;                       // Window attributes (deprecated)
    uint16_t granularity;                           // Window granularity (deprecated)
    uint16_t window_size;                           // Window size in KB
    uint16_t window_start_segment_a;                // Start segment of window A
    uint16_t window_start_segment_b;                // Start segment of window B
    uint32_t window_positioning_function_addr;      // Used to switch banks from protected mode without returning to real mode (deprecated)
    uint16_t bytes_per_scan_line;                   // Bytes per horizontal line
    
    uint16_t width;                                 // Width of screen in pixels
    uint16_t height;                                // Height of screen in pixels
    uint8_t char_cell_width;                        // Width of character in a cell
    uint8_t char_cell_height;                       // Height of character in a cell
    uint8_t memory_plane_count;
    uint8_t bpp;                                    // Bits per pixel of the current mode (e.g. 32 for RGBA8)
    uint8_t bank_count;                             // Number of banks (deprecated)
    uint8_t memory_model_type;                      // Memory model type (see VBE_MemoryModelType)
    uint8_t bank_size_kb;                           // Size of a bank in KB (deprecated)
    uint8_t image_page_count;                       // No. of image pages that will fit into RAM
    uint8_t reserved;                               // 0x00 for VBE 1.0-2.0, 0x01 for VBE 3.0

    // VBE 1.2+

    uint8_t red_mask_size;                          // Size of red colour mask
    uint8_t red_position;                           // Position of the red colour mask
    uint8_t green_mask_size;                        // Size of green colour mask
    uint8_t green_field_size;                       // Size of green field 
    uint8_t blue_mask_size;                         // Size of blue colour mask
    uint8_t blue_field_size;                        // Size of blue field
    uint8_t reserved_mask_size;                     // Size of reserved mask
    uint8_t reserved_mask_position;                 // Position of the reserved mask
    uint8_t colour_mode_info;                       // Direct colour mode info (bit 0 = can programme colour mask, bit 1 = can use reserved bytes)

    // VBE 2.0+

    uint32_t video_buffer_ptr;                      // Physical address of linear video buffer
    uint16_t offscreen_mem_ptr[2];                  // Pointer to the beginning of offscreen video memory
    uint16_t offscreen_mem_size;                    // Size of the offscreen video memory in KB

    // VBE 3.0

    uint16_t linear_bytes_per_scan_line;            // Bytes per scan line (in linear mode)
    uint8_t banked_image_count;                     // Image count for banked video modes (deprecated)
    uint8_t linear_image_count;                     // Image count for linear video modes
    uint8_t red_colour_mask_size;                   // Size of direct red colour mask in bits
    uint8_t lsb_red_mask_position;                  // bit position of LSB for red mask (shift count)
    uint8_t green_colour_mask_size;                 // Size of direct green colour mask in bits
    uint8_t lsb_green_mask_position;                // bit position of LSB for green mask (shift count)
    uint8_t blue_colour_mask_size;                  // Size of direct blue colour mask in bits
    uint8_t lsb_blue_mask_position;                 // bit position of LSB for blue mask (shift count)
    uint8_t reserved_colour_mask_size;              // Size of direct reserved colour mask in bits
    uint8_t lsb_reserved_mask_position;             // bit position of LSB for reserved mask (shift count)
    uint32_t max_pixel_clock_speed;                 // Maximum pixel clock for graphics mode in Hz
    uint8_t reserved_2[190];
} __attribute__((packed)) VBE_VideoModeInfo;


// EDID graphics. Obtained once, need to support modes that can actually be used.
typedef struct {
    uint8_t padding;                            // Unused
    uint16_t manufacture_id;                    // Manufacturer's ID (big-endian)
    uint16_t edid_id_code;                      // EDID ID code
    uint32_t serial_number;                     // Device serial no.
    uint8_t manufacture_week;                   // Device manufacturing week
    uint8_t manufacture_year;                   // Device manufacturing year
    uint8_t video_input_type;                   // Video input type byte
    uint8_t max_horizontal_size;                // Max horizonal size in cm
    uint8_t max_verical_size;                   // Max vertical size in cm
    uint8_t gamma_factor;                       // Gamma factor of the monitor
    uint8_t dpms_flags;                         // Flags for DPMS
    uint8_t chroma_info[10];                    // Chroma information
    uint8_t established_timings_1;              // Established timings, part 1
    uint8_t established_timings_2;              // Established timings, part 2
    uint8_t manufacture_reserved_timings;       // Manufacturer-specific reserved timings
    uint16_t standard_timing_id[8];             // Standard timing identification
    uint8_t detail_desc_1[18];                  // Timing description 1 (detailed)
    uint8_t detail_desc_2[18];                  // Timing description 2
    uint8_t detail_desc_3[18];                  // Timing description 3
    uint8_t detail_desc_4[18];                  // Timing description 4
    uint8_t unused;                             // Unused
    uint8_t checksum;                           // Valid checksum (is lowe byte 16-bit sum of 00-0x7e)
} __attribute__((packed)) EDID_RecordBlock;

typedef enum {
    VBE_SUPPORTED = 1<< 0,
    VBE_OPTIONAL_INFO = 1 << 1,
    VBE_BIOS_OUTPUT = 1 << 2,
    VBE_COLOUR_OUTPUT = 1 << 3,
    VBE_MODE_GRAPHICS = 1 << 4,

    // VBE 2.0+

    VBE_NOT_VGA_COMPATIBLE = 1 << 5,
    VBE_BANK_SWITCH_UNSUPPORTED = 1 << 6,
    VBE_LINEAR_FRAMEBUFFER_SUPPORTED = 1 << 7,
    VBE_DOUBLESCAN_AVAILABLE = 1 << 8,

    // VBE 3.0

    VBE_INTERLACED_AVAILABLE = 1 << 9,
    VBE_TRIPLEBUFFER_SUPPORTED = 1 << 10,
    VBE_STEREOSCOPIC_DISPLAY_SUPPORTED = 1 << 11,
    VBE_DUAL_DISPLAY_START_ADDRESS_SUPPORTED = 1 << 12
} VBE_FramebufferAttribs;

typedef enum {
    MODEL_TEXT = 0x0,
    MODEL_CGA_GRAPHICS = 0x01,
    MODEL_HGC_GRAPHICS = 0x02,
    MODEL_EGA_GRAPHICS = 0x03,
    MODEL_PACKED_PIXEL_GRAPHICS = 0x04,
    MODEL_SEQU_256_GRAPHICS = 0x05,
    MODEL_DIRECT_COLOUR_GRAPHICS = 0x06,
    MODEL_YUV_GRAPHICS = 0x07,
    // Check if greater than when choosing graphics options
    MODEL_OEM_CUSTOM = 0x10
} VBE_MemoryModelType;


#define DESIRED_FRAMEBUFFER_WIDTH 320
#define DESIRED_FRAMEBUFFER_HEIGHT 200
#define DESIRED_FRAMEBUFFER_BPP 32

#define DIFF(x, y) x >= y ? x - y : y - x

VBE_InfoBlock a_info_block;
VBE_VideoModeInfo a_video_info;

EDID_RecordBlock a_record_block;

bool VESA_get_framebuffer(VESA_FramebufferMap *p_out_framebuffer) {
    // Get EDID block first
    if (!x86_EDID_GetVideoBlock(&a_record_block)) {          // Returns true if failed.
        printf("VESA: Could not obtain EDID information.\n");
        return false;
    }

    int x = a_record_block.detail_desc_1[2] | ((int) (a_record_block.detail_desc_1[4] | 0xf0) << 4);
    int y = a_record_block.detail_desc_1[5] | ((int) (a_record_block.detail_desc_1[7] | 0xf0) << 4);

    if (x < DESIRED_FRAMEBUFFER_WIDTH || y < DESIRED_FRAMEBUFFER_HEIGHT) {
        printf("Could not get a valid aspect ratio; wanted %dx%d, got %dx%d\n", DESIRED_FRAMEBUFFER_WIDTH, DESIRED_FRAMEBUFFER_HEIGHT, x, y);
        return false;
    }

    a_info_block.vbe_signature[0] = 'V';
    a_info_block.vbe_signature[1] = 'B';
    a_info_block.vbe_signature[2] = 'E';
    a_info_block.vbe_signature[3] = '2';        // Needed for VBE 2.0 and up

    VESA_Framebuffer fb;

    if (!x86_VBE_GetVESAInfo(&a_info_block)) {
        printf("VESA: Could not obtain VESA VBE information.\n");
        return false;
    }

    if (a_info_block.vbe_version < 0x200) {
        printf("VESA: Version hex %x is too low to be usable.\n", a_info_block.vbe_version);
        return false;
    }

    uint16_t *video_ptr = (uint16_t *)segofs_to_linear(a_info_block.video_mode_ptr[1], a_info_block.video_mode_ptr[0]);
    const char *vendor_name = (const char *)segofs_to_linear(a_info_block.vendor_name_ptr[1], a_info_block.vendor_name_ptr[0]);
    const char *product_name = (const char *)segofs_to_linear(a_info_block.product_name_ptr[1], a_info_block.product_name_ptr[0]);
    
    uint16_t best_x, best_x_diff;
    uint16_t best_y, best_y_diff;
    uint16_t best_depth, best_depth_diff;
    uint16_t a_closest_mode;

    while (*video_ptr != 0xFFFF) {
        // Check if the function failed or not
        if (!x86_VBE_GetVESAVideoModeInfo(*video_ptr, &a_video_info)) {
            printf("VESA: Looking up video mode %d failed.\n", *video_ptr);
            video_ptr++;    // Assume that the other modes will work
            continue;
        }

        if (!(a_video_info.attributes & VBE_SUPPORTED)) {
            printf("VESA: Video mode %d not supported by current hardware configuration.\n", *video_ptr);
            video_ptr++;
            continue;
        }

        if (!(a_video_info.attributes & VBE_OPTIONAL_INFO)) {
            printf("VESA: Video mode %d does not have any optional information, skipping...\n", *video_ptr);
            video_ptr++;
            continue;
        }

        if (!(a_video_info.attributes & VBE_LINEAR_FRAMEBUFFER_SUPPORTED)) {
            printf("VESA: Video mode %d does not have a linear framebuffer, skipping...\n", *video_ptr);
            video_ptr++;
            continue;
        }

        // Exact match
        if (a_video_info.width == DESIRED_FRAMEBUFFER_WIDTH && a_video_info.height == DESIRED_FRAMEBUFFER_HEIGHT && a_video_info.bpp == DESIRED_FRAMEBUFFER_BPP) {
            a_closest_mode = *video_ptr;
            best_x = DESIRED_FRAMEBUFFER_WIDTH;
            best_y = DESIRED_FRAMEBUFFER_HEIGHT;
            best_depth = DESIRED_FRAMEBUFFER_BPP;
            printf("Video driver at %dx%dx%d, address %x\n", a_video_info.width, a_video_info.height, a_video_info.bpp, a_video_info.video_buffer_ptr);
            break;
        }

        // Otherwise, make it a closer match
        uint16_t xdiff = (a_video_info.width >= DESIRED_FRAMEBUFFER_WIDTH) ? 
                a_video_info.width - DESIRED_FRAMEBUFFER_WIDTH : DESIRED_FRAMEBUFFER_WIDTH - a_video_info.width;
        uint16_t ydiff = (a_video_info.height >= DESIRED_FRAMEBUFFER_HEIGHT) ? 
                a_video_info.height - DESIRED_FRAMEBUFFER_HEIGHT : DESIRED_FRAMEBUFFER_HEIGHT - a_video_info.height;
        uint16_t bppdiff = (a_video_info.bpp >= DESIRED_FRAMEBUFFER_BPP) ? 
                a_video_info.bpp - DESIRED_FRAMEBUFFER_BPP : DESIRED_FRAMEBUFFER_BPP - a_video_info.bpp;
        
        if (xdiff < best_x_diff) {
            best_x_diff = xdiff;
            best_x = a_video_info.width;
        }

        if (ydiff < best_y_diff) {
            best_y_diff = ydiff;
            best_y = a_video_info.height;
        }

        if (bppdiff < best_depth_diff) {
            best_depth_diff = bppdiff;
            best_depth = a_video_info.bpp;
        }
        
        a_closest_mode = *video_ptr;
        video_ptr++;
    }

    printf("VESA: Red mask size: %d, offset %d\n Green mask size: %d, offset %d\nBlue mask size: %d, offset %d\n", 
                a_video_info.red_colour_mask_size, a_video_info.lsb_red_mask_position,
                a_video_info.green_colour_mask_size, a_video_info.lsb_green_mask_position,
                a_video_info.blue_colour_mask_size, a_video_info.lsb_blue_mask_position);
    printf("VESA: Reserved size: %d, offset %d\n", a_video_info.reserved_colour_mask_size, a_video_info.lsb_reserved_mask_position);

    if (a_closest_mode == 0) {
        printf("VESA: Failed to find a successful video mode.\n");
        return false;
    }
    
    if (!x86_VBE_GetVESAVideoModeInfo(a_closest_mode, &a_video_info)) {
        printf("Are you kidding?!\n");
        return false;
    }
    printf("Loaded driver ID %d\n", a_closest_mode);
    printf("\tXRES: %d\n\tYRES: %d\n\tBPP: %d\n\tADDR: %x\n", a_video_info.width, a_video_info.height, a_video_info.bpp, a_video_info.video_buffer_ptr);

    fb.address = a_video_info.video_buffer_ptr;
    fb.width = a_video_info.width;
    fb.height = a_video_info.height;
    fb.bpp = a_video_info.bpp;
    fb.bytes_per_line = a_video_info.bytes_per_scan_line;
    fb.mode_id = a_closest_mode;

    p_out_framebuffer->framebuffer = fb;
    p_out_framebuffer->vendor_name = vendor_name;
    p_out_framebuffer->product_name = product_name;

    printf("VESA: Using GPU %s from vendor %s, found memory at address %x.\n", vendor_name, product_name, fb.address);   
    printf("VESA: Running VESA VBE version %x\n", a_info_block.vbe_version);
    return true;
}