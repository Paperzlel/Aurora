#include "floppy.h"
#include "../pic.h"

#include <kernel/arch/arch.h>
#include <kernel/arch/io.h>

typedef enum {
    REGISTER_STATUS_A           = 0x3f0,        // read-only, status of drive A (SRA)
    REGISTER_STATUS_B           = 0x3f1,        // read-only, status of drive B (SRB)
    REGISTER_DIGITAL_OUTPUT     = 0x3f2,        // read-write, drive command controller (DOR)
    REGISTER_TAPE_DRIVE         = 0x3f3,        // read-write, assigns tape support to a drive (TDR). Unused by pretty much everything.
    REGISTER_MAIN_STATUS        = 0x3f4,        // read-only, overall controller status (MSR)
    REGISTER_DATARATE_SELECTOR  = 0x3f4,        // write-only, rate of data transfer (DSR)
    REGISTER_DATA_FIFO          = 0x3f5,        // read-write, all commands go here (FIFO)
    REGISTER_DIGITAL_INPUT      = 0x3f7,        // read-only, digital input (DIR)
    REGISTER_CONFIG_CONTROL     = 0x3f7         // write-only, controls the config (CCR)
} FloppyRegisters;

typedef enum {
    DOR_MOTD    = 0x80,     // Turn on drive 3's motor
    DOR_MOTC    = 0x40,     // Turn on drive 2's motor
    DOR_MOTB    = 0x20,     // Turn on drive 1's motor
    DOR_MOTA    = 0x10,     // Turn on drive 0's motor
    DOR_IRQ     = 0x8,      // Enables IRQs and DMA transfers
    DOR_RESET   = 0x4,      // When cleared, enters reset mode (OSDev nots that we should use DSR instead because of hardware stuff)
    DOR_DSELD   = 0x3,      // Select drive 3
    DOR_DSELC   = 0x2,      // Select drive 2
    DOR_DSELB   = 0x1,      // Select drive 1
    DOR_DSELA   = 0         // Select drive 0
} DOR_Bitmask;

typedef enum {
    MSR_RQM     = 0x80,     // Sets if we can exchange bytes with the FIFO IO port
    MSR_DIO     = 0x40,     // Sets if FIFO IO ports expect an IN opcode
    MSR_NDMA    = 0x20,     // Sets when PIO execution is in read/write mode
    MSR_CB      = 0x10,     // Sets when the command byte is recieved and cleared at the end of the "results" phase
    MSR_ACTD    = 0x8,      // Drive 3 is seeking
    MSR_ACTC    = 0x4,      // Drive 2 is seeking
    MSR_ACTB    = 0x2,      // Drive 1 is seeking
    MSR_ACTA    = 0x1       // Drive 0 is seeking
} MSR_Bitmask;

typedef enum {
    FLOPPY_READ_TRACK           = 2,    // Generates IRQ6, don't use
    FLOPPY_SPECIFY              = 3,    // Set drive parameters
    FLOPPY_SENSE_DRIVE_STATUS   = 4,    // Get drive status, don't use
    FLOPPY_WRITE_DATA           = 5,    // Write data to the disk (get DMA ready first)
    FLOPPY_READ_DATA            = 6,    // Read data from the disk (ready DMA first)
    FLOPPY_RECALIBRATE          = 7,    // Seek back to cylinder 0
    FLOPPY_SENSE_INTERRUPT      = 8,    // Get last command status in IRQ6
    FLOPPY_WRITE_DELETED_DATA   = 9,    // Deprecated, don't use
    FLOPPY_READ_ID              = 10,   // Reads floppy disk ID (?), don't use
    FLOPPY_READ_DELETED_DATA    = 12,   // Deprecated, don't use
    FLOPPY_FORMAT_TRACK         = 13,   // Formats a track, which I don't know what that does
    FLOPPY_DUMPREG              = 14,   // Gets error information if needed. Don't use
    FLOPPY_SEEK                 = 15,   // Seek heads to cylinder n
    FLOPPY_VERSION              = 16,   // Set information in initialization
    FLOPPY_SCAN_EQUAL           = 17,   // Don't use
    FLOPPY_PERPENDICULAR_MODE   = 18,   // Set when using 2.88M drives, otherwise don't use
    FLOPPY_CONFIGURE            = 19,   // Set controller parameters
    FLOPPY_LOCK                 = 20,   // Protect parameters during controller reset
    FLOPPY_VERIFY               = 22,   // Don't use
    FLOPPY_SCAN_LOW_OR_EQUAL    = 25,   // Don't use
    FLOPPY_SCAN_HIGH_OR_EQUAL   = 29    // Don't use
} FloppyCommands;

#define BIT_MULTITRACK 0x80
#define BIT_MFM 0x40

// Data rates for the DSR/CCR (can set either one if needed)

// Data rate for 2.88M floppies
#define DATARATE_1MBPS 3
// Data rate for 1.44M/1.2M floppies
#define DATARATE_500KBPS 0

// Custom data structures

typedef struct {
    bool exists;
    uint8_t dsr_value;
    uint8_t step_rate_head_unload;
    uint8_t head_load_use_dma;
} FloppyDrive;

typedef struct {
    uint8_t drive_count;
    FloppyDrive drives[2];
} FloppyConfig;

static FloppyConfig fc;

void floppy_write_command(char command) {
    // Check every N times to see if the drive is up to speed.
    for (int i = 0; i < 120; i++) {
        // TODO: Sleep current thread function
        if (inb(REGISTER_MAIN_STATUS) & 0x80) {
            outb(REGISTER_DATA_FIFO, command);
            return;
        }
    }
    
    // TODO: Print error
}

uint8_t floppy_read_data() {
    for (int i = 0; i < 120; i++) {
        // TODO: sleep
        if (inb(REGISTER_MAIN_STATUS) & 0x80) {
            return inb(REGISTER_DATA_FIFO);
        }
    }
    // TODO: print error
    return 0;
}

static volatile bool reset_handled = 0;

bool floppy_disk_handler(Registers *p_regs) {
    reset_handled = true;

    arch_send_eoi(p_regs->interrupt);
    return true;
}

void floppy_drive_reset(uint8_t drive_id) {
    // Prevent race condition with ISR
    reset_handled = false;
    // Set reset flag in DSR plus the drive info
    outb(REGISTER_DATARATE_SELECTOR, fc.drives[drive_id].dsr_value | 0x80);

    while (!reset_handled);

    outb(REGISTER_CONFIG_CONTROL, fc.drives[drive_id].dsr_value);
    // Send specify command
    floppy_write_command(FLOPPY_SPECIFY);
    floppy_write_command(fc.drives[drive_id].step_rate_head_unload);
    floppy_write_command(fc.drives[drive_id].head_load_use_dma);
    

    outb(REGISTER_DIGITAL_OUTPUT, drive_id | DOR_IRQ);
}

/**
 * @brief Moves the drive back to cylinder 0. Needs the drive's motor to be enabled and the drive to be selected.
 * @param drive_id The drive to run.
 */
void floppy_recalibrate(uint8_t drive_id) {
    reset_handled = false;
    floppy_write_command(FLOPPY_RECALIBRATE);
    floppy_write_command(drive_id);

    while (!reset_handled);

    floppy_write_command(FLOPPY_SENSE_INTERRUPT);
    (void) floppy_read_data();
    (void) floppy_read_data();
}

void floppy_initialize() {
    // Setup IRQ6 (floppy disk controller)
    arch_register_isr_handler(0x26, floppy_disk_handler);
    pic_unmask_irq(0x26);

    // Find the number of floppy drives attached
    outb(0x70, 1 << 7 | 0x10);
    uint8_t drive_info = inb(0x71);
    fc.drive_count = (drive_info & 0x0f) > 0 ? 2 : 1;
    fc.drives[0].exists = true;
    fc.drives[0].dsr_value = (drive_info >> 4) == 5 ? 3 : 0;
    fc.drives[1].exists = fc.drive_count == 2;
    fc.drives[1].dsr_value = (drive_info & 0x0f) == 5 ? 3 : 0;

    floppy_write_command(FLOPPY_VERSION);
    if (floppy_read_data() != 0x90) {
        return;
    }

    // Configure device
    floppy_write_command(FLOPPY_CONFIGURE);
    floppy_write_command(0);
    // Enable (LTR) implied seeks ON, FIFO ON, drive polling OFF, threshold 8
    floppy_write_command((1 << 6) | (1 << 5) | 0 << 4 | 8);
    // Disable precompensation
    floppy_write_command(0);

    // Lock FIFO configuration so we don't re-enable it on reset
    floppy_write_command(FLOPPY_LOCK | BIT_MULTITRACK);
    if (floppy_read_data() != (1 << 4)) {
        return;
    }

    // Set drive info
    for (int i = 0; i < fc.drive_count; i++) {
        // 8ms step rate time, 10ms head load time
        // SRT = 16 - (ms * data_rate / 500000)
        // HUT = 24 * data_rate / 8000000
        fc.drives[i].step_rate_head_unload = 8 << 4 | 0;
        // 10ms head load time, use DMA
        // HLT = ms * data_rate / 1000000
        fc.drives[i].head_load_use_dma = 5 << 4 | 1;

        floppy_drive_reset(i);
        // Only recalibrate if the drives are moving. We want to preserve the drives, so keep 'em static for now.
        if (inb(REGISTER_DIGITAL_OUTPUT) & (1 << (4 + i))) {
            floppy_recalibrate(i);
        }
    }
}