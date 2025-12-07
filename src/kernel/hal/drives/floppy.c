#include "floppy.h"
#include "floppy_dma.h"

#include <aurora/arch/interrupts.h>
#include <asm/io.h>

#include <sys/time.h>

#define AUR_MODULE "floppy"
#include <aurora/debug.h>

enum FloppyRegisters
{
    REGISTER_STATUS_A           = 0x3f0,        // read-only, status of drive A (SRA)
    REGISTER_STATUS_B           = 0x3f1,        // read-only, status of drive B (SRB)
    REGISTER_DIGITAL_OUTPUT     = 0x3f2,        // read-write, drive command controller (DOR)
    REGISTER_TAPE_DRIVE         = 0x3f3,        // read-write, assigns tape support to a drive (TDR). Unused by pretty much everything.
    REGISTER_MAIN_STATUS        = 0x3f4,        // read-only, overall controller status (MSR)
    REGISTER_DATARATE_SELECTOR  = 0x3f4,        // write-only, rate of data transfer (DSR)
    REGISTER_DATA_FIFO          = 0x3f5,        // read-write, all commands go here (FIFO)
    REGISTER_DIGITAL_INPUT      = 0x3f7,        // read-only, digital input (DIR)
    REGISTER_CONFIG_CONTROL     = 0x3f7         // write-only, controls the config (CCR)
};

enum DOR_Bitmask
{
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
};

enum MSR_Bitmask
{
    MSR_RQM     = 0x80,     // Sets if we can exchange bytes with the FIFO IO port
    MSR_DIO     = 0x40,     // Sets if FIFO IO ports expect an IN opcode
    MSR_NDMA    = 0x20,     // Sets when PIO execution is in read/write mode
    MSR_CB      = 0x10,     // Sets when the command byte is recieved and cleared at the end of the "results" phase
    MSR_ACTD    = 0x8,      // Drive 3 is seeking
    MSR_ACTC    = 0x4,      // Drive 2 is seeking
    MSR_ACTB    = 0x2,      // Drive 1 is seeking
    MSR_ACTA    = 0x1       // Drive 0 is seeking
};

enum FloppyCommands
{
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
};

#define BIT_MULTITRACK 0x80
#define BIT_MFM 0x40

// Data rates for the DSR/CCR (can set either one if needed)

// Data rate for 2.88M floppies
#define DATARATE_1MBPS 3
// Data rate for 1.44M/1.2M floppies
#define DATARATE_500KBPS 0

// Custom data structures

struct FloppyDrive
{
    bool exists;
    uint8_t dsr_value;
    uint8_t step_rate_head_unload;
    uint8_t head_load_use_dma;
};

struct FloppyConfig
{
    uint8_t drive_count;
    uint8_t sectors;
    uint8_t heads;
    uint8_t current_drive;
    struct FloppyDrive drives[2];
    bool initialized;
};

static struct FloppyConfig fc;

static volatile bool irq_handled = 0;

static void floppy_drive_reset(uint8_t drive_id);

bool floppy_disk_handler(struct Registers *p_regs)
{
    irq_handled = true;

    send_end_of_interrupt(p_regs->interrupt);
    return true;
}

static void floppy_write_command(char command)
{
    // Check every N times to see if the drive is up to speed.
    for (int i = 0; i < 120; i++)
    {
        timer_sleep(2);             // Wait 2ms until issuing another command
        if ((inb(REGISTER_MAIN_STATUS) & (MSR_RQM | MSR_DIO)) == MSR_RQM)
        {
            outb(REGISTER_DATA_FIFO, command);
            return;
        }
    }
    
    LOG_ERROR("Controller timed out, waited too long on command %hhx.", command);
}

static uint8_t floppy_read_data()
{
    while ((inb(REGISTER_MAIN_STATUS) & (MSR_RQM | MSR_DIO)) != (MSR_RQM | MSR_DIO));

    for (int i = 0; i < 120; i++)
    {
        timer_sleep(2);
        if (inb(REGISTER_MAIN_STATUS) & MSR_RQM)
        {
            return inb(REGISTER_DATA_FIFO);
        }
    }
    
    LOG_ERROR("Controller timed out, waited too long for data to arrive.");
    return 0;
}

static void floppy_drive_reset(uint8_t drive_id)
{
    // Prevent race condition with IRQ 6
    irq_handled = false;
    // Set reset flag in DSR plus the drive info
    outb(REGISTER_DATARATE_SELECTOR, fc.drives[drive_id].dsr_value | 0x80);

    while (!irq_handled);

    // Check for interrupt
    floppy_write_command(FLOPPY_SENSE_INTERRUPT);
    while ((inb(REGISTER_MAIN_STATUS) & MSR_RQM) == 0);
    (void) floppy_read_data();
    (void) floppy_read_data();

    outb(REGISTER_CONFIG_CONTROL, fc.drives[drive_id].dsr_value);
    // Send specify command
    floppy_write_command(FLOPPY_SPECIFY);
    floppy_write_command(fc.drives[drive_id].step_rate_head_unload);
    floppy_write_command(fc.drives[drive_id].head_load_use_dma);
}

/**
 * @brief Moves the drive back to cylinder 0. Needs the drive's motor to be enabled and the drive to be selected.
 * @param drive_id The drive to run.
 */
static void floppy_recalibrate(uint8_t drive_id)
{
    for (int i = 0; i < 3; i++) {
        irq_handled = false;
        floppy_write_command(FLOPPY_RECALIBRATE);
        floppy_write_command(drive_id);

        while (!irq_handled);

        floppy_write_command(FLOPPY_SENSE_INTERRUPT);
        uint8_t st0 = floppy_read_data();
        (void) floppy_read_data();

        if (st0 & (1 << 5))
        {
            return;
        }
    }

    LOG_ERROR("Controller failed to recalibrate, disk reset is required.");
}

static void floppy_lba_to_chs(uint16_t lba, uint16_t *cylinder, uint16_t *sector, uint16_t *head)
{
    *cylinder = lba / (fc.sectors * fc.heads);
    *head = (lba % (fc.sectors * fc.heads)) / fc.sectors;
    *sector = (lba % fc.sectors) + 1;
}

static bool floppy_drive_begin_rw(uint8_t drive_id, uint16_t lba, void *start, size_t size, bool is_write)
{
    // Start up the motor
    uint8_t dor = inb(REGISTER_DIGITAL_OUTPUT);
    outb(REGISTER_DIGITAL_OUTPUT, dor | (DOR_MOTA << drive_id) | (drive_id > 0 ? DOR_DSELB << (drive_id - 1) : DOR_DSELA));
    timer_sleep(50);        // Wait 50ms to turn the motor on
    
    // Check if current drive is set
    if (fc.current_drive != drive_id)
    {
        outb(REGISTER_CONFIG_CONTROL, fc.drives[drive_id].dsr_value);
    }
    
    // Specify the new drive
    floppy_write_command(FLOPPY_SPECIFY);
    floppy_write_command(fc.drives[drive_id].step_rate_head_unload);
    floppy_write_command(fc.drives[drive_id].head_load_use_dma);
    
    
    uint16_t cylinder, sector, head;
    floppy_lba_to_chs(lba, &cylinder, &sector, &head);
    
    // Seek + sense interrupt
    floppy_write_command(FLOPPY_SEEK);
    floppy_write_command((head << 2) | drive_id);
    floppy_write_command(cylinder);

    while (!irq_handled);

    floppy_write_command(FLOPPY_SENSE_INTERRUPT);

    while ((inb(REGISTER_MAIN_STATUS) & MSR_RQM) == 0);

    uint8_t st0 = floppy_read_data();
    (void) floppy_read_data();

    if (st0 != (0x20 | drive_id))
    {
        LOG_ERROR("Seek command failed, got 0x%hhx, expected 0x%hhx.", st0, (0x20 | drive_id));
    }

    fc.current_drive = drive_id;
    floppy_dma_setup_for_location(start, size);
    if (is_write)
    {
        floppy_dma_write();
    }
    else
    {
        floppy_dma_read();
    }

    for (int i = 0; i < 3; i++)
    {
        irq_handled = false;
        // Ready for reading
        floppy_write_command((is_write ? FLOPPY_WRITE_DATA : FLOPPY_READ_DATA) | BIT_MULTITRACK | BIT_MFM);
        floppy_write_command((head << 2) | drive_id);
        floppy_write_command(cylinder);
        floppy_write_command(head);
        floppy_write_command(sector);
        floppy_write_command(2);
        floppy_write_command(fc.sectors);
        floppy_write_command(0x1b);
        floppy_write_command(0xff);

        while (!irq_handled);
        while (!(inb(REGISTER_MAIN_STATUS) & (MSR_RQM | MSR_DIO)));

        st0 = floppy_read_data();
        uint8_t st1 = floppy_read_data();
        uint8_t st2 = floppy_read_data();
        (void) floppy_read_data();
        (void) floppy_read_data();
        (void) floppy_read_data();
        uint8_t two = floppy_read_data();

        if (two != 2)
        {
            continue;
        }

        if ((st0 & 0x80) || (st0 & 0x40))
        {
            continue;
        }

        if (st1 & 0x80)
        {
            LOG_ERROR("Insufficient sector count to complete the read/write operation.");
            return false;
        }

        if (st1 & 0x10)
        {
            LOG_ERROR("Driver took too long to get bytes in and out of the FIFO port.");
            continue;
        }

        if (st1 & 0x02)
        {
            LOG_ERROR("Media is write-protected, unable to write.");
            return false;
        }

        if (st2 != 0)
        {
            LOG_ERROR("Potential bad drive/media problems.");
            continue;
        }

        return true;
    }

    LOG_ERROR("Controller timed out on read/write operation.");
    return false;
}

void floppy_initialize()
{
    // Set IRQ6 (floppy disk controller) and immediately enable for use
    register_interrupt_handler(INT_IRQ_6, floppy_disk_handler);
    unmask_irq(INT_IRQ_6);

    // Find the number of floppy drives attached
    outb(0x70, 1 << 7 | 0x10);
    uint8_t drive_info = inb(0x71);
    fc.drive_count = (drive_info & 0x0f) > 0 ? 2 : 1;
    fc.drives[0].exists = true;
    fc.drives[0].dsr_value = (drive_info >> 4) == 5 ? 3 : 0;
    fc.drives[1].exists = fc.drive_count == 2;
    fc.drives[1].dsr_value = (drive_info & 0x0f) == 5 ? 3 : 0;

    // Get heads per disk and sectors per track
    fc.sectors = *(uint16_t *)(0x7c00 + 0x18);
    fc.heads = *(uint16_t *)(0x7c00 + 0x1a);

    floppy_write_command(FLOPPY_VERSION);
    if (floppy_read_data() != 0x90)
    {
        LOG_ERROR("Floppy controller is not an 82077AA FDC. This drive type is unsupported and any attempts to read/write will fall through.");
        return;
    }

    // Configure device
    floppy_write_command(FLOPPY_CONFIGURE);
    floppy_write_command(0);
    // Enable (LTR) implied seeks ON, FIFO ON, drive polling OFF, threshold 8
    floppy_write_command((1 << 6) | (0 << 5) | 1 << 4 | 9);
    // Disable precompensation
    floppy_write_command(0);

    // Lock FIFO configuration so we don't re-enable it on reset
    floppy_write_command(FLOPPY_LOCK | BIT_MULTITRACK);
    if (floppy_read_data() != (1 << 4))
    {
        return;
    }

    // Enable DMA now that the FDC is hopefully ready to work
    uint8_t dor = inb(REGISTER_DIGITAL_OUTPUT);
    outb(REGISTER_DIGITAL_OUTPUT, dor | DOR_IRQ);

    // Set drive info
    for (int i = 0; i < fc.drive_count; i++)
    {
        // 8ms step rate time, 10ms head load time
        // SRT = 16 - (ms * data_rate / 500000)
        // HUT = 24 * data_rate / 8000000
        fc.drives[i].step_rate_head_unload = 8 << 4 | 0;
        // 10ms head load time, use DMA
        // HLT = ms * data_rate / 1000000
        fc.drives[i].head_load_use_dma = 5 << 4 | 0;

        floppy_drive_reset(i);
        uint8_t dor = inb(REGISTER_DIGITAL_OUTPUT);
        // Only recalibrate if the drives are moving, and if the drive is selected. We want to preserve the drives, so keep 'em static for now.
        if (dor & (1 << (4 + i)))
        {
            // Manually select drive, just to be sure.
            outb(REGISTER_DIGITAL_OUTPUT, dor | (i > 0 ? DOR_DSELB + (i - 1) : DOR_DSELA));
            floppy_recalibrate(i);
        }
    }

    fc.initialized = true;
    uint8_t *data = (uint8_t *)floppy_read(0, 0, (void *)0x7e00, 512);
    LOG_DEBUG("Data as string is the following: %s", (const char *)data);
}

void *floppy_read(uint8_t p_drive, uint16_t p_lba, void *p_to, size_t p_size)
{
    if (!fc.initialized)
    {
        LOG_ERROR("Attempted to read information prior to initializing the floppy disk driver, or that the floppy is unsupported.");
        return NULL;
    }

    if (fc.drive_count < p_drive)
    {
        LOG_ERROR("Drive ID does not exist.");
        return NULL;
    }

    if (floppy_drive_begin_rw(p_drive, p_lba, p_to, p_size, false))
    {
        return p_to;
    }

    LOG_ERROR("Failed to read information to disk.");
    return NULL;
}

bool floppy_write(uint8_t p_drive, uint16_t p_lba, void *p_from, size_t p_size)
{
    if (!fc.initialized)
    {
        LOG_ERROR("Attempted to write information prior to initializing the floppy disk driver, or that the floppy is unsupported.");
        return false;
    }

    if (fc.drive_count < p_drive)
    {
        LOG_ERROR("Drive ID does not exist.");
        return false;
    }

    if (floppy_drive_begin_rw(p_drive, p_lba, p_from, p_size, true))
    {
        return true;
    }

    LOG_ERROR("Failed to write information to disk.");
    return false;
}
