#include "apic.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/arch/io.h>

// RSDP/XSDP

typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed)) RSDP;

typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) XSDP;

char signature[8] = { 'R', 'S', 'D', ' ', 'P', 'T', 'R', ' ' };
char rsdp_oem[6];

#define SEARCH_BACKUP_START (uint8_t *)0xe0000

// RSDT/XSDT

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) RSDT_Header;

typedef struct {
    RSDT_Header header;
    uint32_t other_tables[];             // Pointer to the other ACPI tables. Amount is read from the header.
} __attribute__((packed)) RSDT;

// MADT

typedef struct {
    uint8_t entry_type;
    uint8_t record_length;
    uint8_t data[14];
} __attribute__((packed)) MADT_Data;

typedef struct {
    RSDT_Header header;
    uint32_t local_apic_address;
    uint32_t flags;
    MADT_Data *data;
} __attribute__((packed)) MADT;

// CPU information

#define IA32_APIC_BASE_MSR 0x1b
#define IA32_APIC_BASE_MSR_BSP 0x100
#define IA32_APIC_BASE_MSR_ENABLE 0x800

typedef struct {
    void *local_apic_address;
    int acpi_processor_id;
} CPU_Information;

CPU_Information cpu_info;

// Other tables
typedef enum {
    RSDT_MADT,
    RSDT_FADT,
    RSDT_DSDT,
    RSDT_SSDT,
} RSDT_TableIndex;


char table_names[][5] = {
    "APIC",
    "FACP",
    "DSDT",
    "SSDT",
};


bool check_rsdp_checksum(RSDP *p_table) {
    uint32_t sum = 0;
    int i = 0;
    for (i = 0; i < 8; i++) {
        sum += p_table->signature[i];
    }

    for (i = 0; i < 6; i++) {
        sum += p_table->oem_id[i];
    }

    for (i = 0; i < 4; i++) {
        sum += *((uint8_t *)(&p_table->rsdt_address) + i);
    }

    sum += p_table->revision;
    sum += p_table->checksum;
    if (sum & 0xff) {
        return false;
    }

    return true;
}


bool check_xsdp_checksum(XSDP *p_table) {
    uint32_t sum = 0;
    int i = 0;
    for (i = 0; i < sizeof(XSDP) - sizeof(RSDP); i++) {
        sum += *((uint8_t *)(p_table) + i);
    }

    return (sum & 0xff) == 0;
}


bool check_rsdt_header_checksum(RSDT_Header *p_header) {
    uint32_t sum = 0;
    for (int i = 0; i < sizeof(RSDT_Header); i++) {
        sum += *((uint8_t *)(p_header) + i);
    }

    return (sum & 0xff) == 0;
}


bool apic_get_rsdt(void **out_location) {

    uint32_t ebda_addr = *(uint16_t *)(0x40e) << 4;
    if (!ebda_addr) {
        return false;
    }

    // Check EBDA first
    uint8_t *memory = (uint8_t *)ebda_addr;
    bool found = false;
    while (memory < (uint8_t *)0x9ffff) {
        if (!memcmp(memory, signature, 8)) {
            found = true;
            break;
        }

        memory += 0x10;         // Next 16-byte alignment
    }

    // Check rest of BIOS memory to find it
    if (!found) {
        memory = SEARCH_BACKUP_START;
        while (memory < (uint8_t *)0xfffff) {
            if (!memcmp(memory, signature, 8)) {
                found = true;
                break;
            }

            memory += 0x10;
        }
    }

    // TODO: Throw an error.
    if (!found) {
        return false;
    }

    RSDP sdp_base;
    memcpy(&sdp_base, memory, sizeof(RSDP));

    if (!check_rsdp_checksum(&sdp_base)) {
        return false;
    }

    uint32_t pointer = 0;           // Should really be 64-bit, but we only support x86 for the meantime
    // uint32_t size = 0;

    if (sdp_base.revision == 2) {
        XSDP sdp_v2;
        memcpy(&sdp_v2, memory, sizeof(XSDP));

        // Checksum validation
        if (!check_xsdp_checksum(&sdp_v2)) {
            return false;
        }

        pointer = sdp_v2.xsdt_address;
        // size = sdp_v2.length;
    } else {
        pointer = sdp_base.rsdt_address;
    }

    // Copy in OEM ID
    memcpy(rsdp_oem, sdp_base.oem_id, 6);

    if (!pointer) {
        return false;
    }

    void *address = (void *)pointer;
    *out_location = address;
    return true;
}


bool apic_parse_madt(void *p_madt) {
    MADT *m = (MADT *)p_madt;

    cpu_info.local_apic_address = (void *)m->local_apic_address;
    int bytes_left = m->header.length - sizeof(RSDT_Header);
    int i = 0;
    while (bytes_left > 0) {
        // Read APIC info

        uint8_t type = m->data[i].entry_type;
        switch (type) {
            // Processor-local APIC
            case 0: {
                // See if the processor can be enabled
                if (!(m->data[i].data[2] & 1)) {
                    // Cannot enable the CPU
                    if (!(m->data[i].data[2] & 1)) {
                        break;
                    }
                    
                    m->data[i].data[2] |= 1;      // Enable
                }
            } break;
            // I/O APIC
            case 1: {

            } break;
            default:
                break;
        }

        bytes_left -= m->data[i].record_length;
        i++;
    }

    // Set APIC address
    uint64_t ret = rdmsr(IA32_APIC_BASE_MSR);
    uint32_t eax = ret >> 32;
    eax &= 0xfffff000;
    eax |= IA32_APIC_BASE_MSR_ENABLE;
    wrmsr(IA32_APIC_BASE_MSR, eax);

    // Write SIVR bit 8 to enable interrupts
    uint32_t svir_value = *((volatile uint32_t *)(eax + 0xf0));
    svir_value |= 0x100;
    *((volatile uint32_t *)(eax + 0xf0)) = svir_value;
    return true;
}


bool apic_initialize() {
    // Look for the RSDP

    void *rsdt = NULL;
    if (!apic_get_rsdt(&rsdt)) {
        return false;
    }

    RSDT *root_table = (RSDT *)rsdt;
    if (check_rsdt_header_checksum(&root_table->header)) {
        return false;
    }

    // Look for MADT
    void *madt = NULL;
    int entry_count = (root_table->header.length - sizeof(root_table->header)) / 4;         // If XSDT, should be 8
    for (int i = 0; i < entry_count; i++) {
        RSDT_Header *next = (RSDT_Header *)root_table->other_tables[i];
        
        // if (!check_rsdt_header_checksum(next)) {
        //     continue;
        // }
        
        if (next && memcmp(next, table_names[RSDT_MADT], 4) == 0) {
            madt = (void *)next;
            break;
        }
    }

    if (!madt) {
        return false;
    }

    // Parse MADT
    if (!apic_parse_madt(madt)) {
        return false;
    }

    return true;
}
