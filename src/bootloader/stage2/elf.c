#include "elf.h"

#include "stdio.h"
#include "memory.h"

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t file_size;
    uint32_t memory_size;
    uint32_t flags;
    uint32_t align;
} ELF_ProgramHeader;

typedef enum {
    ELF_NULL = 0,
    ELF_LOAD = 1,
    ELF_DYNAMIC = 2,
    ELF_INTERPRETER = 3,
    ELF_NOTE = 4
} ELF_ProgHeaderType;

typedef struct {
    uint32_t name_offset;
    uint32_t type;
    uint32_t flags;
    uint32_t virtual_address;
    uint32_t offset;
    uint32_t size;
    uint32_t section_link_index;
    uint32_t extra_information;
    uint32_t alignment;
    uint32_t entry_size;
} ELF_SectionHeader;

typedef enum {
    ELF_SECTION_NULL = 0,
    ELF_SECTION_PROGBITS = 1,
    ELF_SECTION_SYMTAB = 2,
    ELF_SECTION_STRTAB = 3,
    ELF_SECTION_RELOCATION = 4,
    ELF_SECTION_HASH_TABLE = 5,
    ELF_SECTION_DYNAMIC = 6,
    ELF_SECTION_NOTES = 7,
    ELF_SECTION_NOBITS = 8,
} ELF_SectionInfo;

typedef enum {
    ELF_TYPE_WRITABLE = 0x1,
    ELF_TYPE_ALLOC = 0x2,
    ELF_TYPE_EXECUTE = 0x4,
    ELF_TYPE_MERGE = 0x10,
    ELF_TYPE_STRINGS = 0x20,
} ELF_SectionFlags;


uint8_t magic[4] = {0x7f, 0x45, 0x4c, 0x46};

bool elf_is_elf(uint8_t *p_file) {
    return memcmp(p_file, &magic, 4) == 0;
}

bool elf_is_valid_format(ELF_Header *p_file) {
    if (p_file->bit_size != ELF_32BIT) {
        // Currently only parsing ELF32
        printf("ELF: Cannot parse 64-bit header files.\n");
        return false;
    }
    
    ELF_Header *elf = p_file;

    if (elf->endianess != ELF_LE) {
        printf("ELF: Endianness is not little-endian, is format %u.\n", elf->endianess);
        return false;
    }

    if (elf->os_abi_target != ELF_SYSTEMV) {
        printf("ELF: ABI format is not set to that of System V, unknown format %x.\n", elf->os_abi_target);
        return false;
    }

    if (elf->object_type != ELF_EXECUTABLE) {
        printf("ELF: Object file type is invalid, unknown format %x.\n", elf->object_type);
    }

    switch (elf->target_isa) {
        case ELF_X86:
        case ELF_POWERPC32:
        case ELF_POWERPC64:
        case ELF_ARM32:
        case ELF_IA64:
        case ELF_AMD64:
        case ELF_ARM64:
        case ELF_RISCV:
            break;
        default:
            printf("ELF: ISA format is invalid, unknown format %x.\n", elf->target_isa);
            return false;
    }

    return true;
}

bool elf_read(ELF_Header *p_header, uint8_t *p_buffer, void **p_out_entry_point) {
    // First load program headers at their desired addresses
    uint16_t header_size = p_header->program_header_entry_size;

    for (int i = 0; i < p_header->program_header_entry_count; i++) {
        ELF_ProgramHeader ph;
        memcpy(&ph, p_buffer + p_header->prog_header_offset + (header_size * i), header_size);

        if (ph.type != ELF_LOAD) {
            printf("ELF: Error parsing program header as its type is not of ELF_LOAD, is type %x.\n", ph.type);
            return false;
        }

        // WARNING: Only do this for physical addresses now, virtual ones will require a separate system.
        memset((uint32_t *)ph.physical_address, 0, ph.memory_size);
        memcpy((uint32_t *)ph.physical_address, p_buffer + ph.offset, ph.memory_size);
    }

    // Pre-emptively get the section header name
    ELF_SectionHeader *sh_name = (ELF_SectionHeader *)(p_buffer + p_header->sect_header_offset + 
                (p_header->section_header_entry_section_names * p_header->section_header_entry_size));

    // Change header size, then start loading section headers
    header_size = p_header->section_header_entry_size;

    for (int i = 0; i < p_header->section_header_entry_count; i++) {
        ELF_SectionHeader sh;
        memcpy(&sh, p_buffer + p_header->sect_header_offset + (header_size * i), header_size);

        // Get section name;
        char *name = p_buffer + sh_name->offset + sh.name_offset;

        if (sh.type == ELF_SECTION_NULL) {
            // printf("ELF: Skipping null header %s\n", name);
            continue;
        }

        if (!(sh.flags & ELF_TYPE_ALLOC)) {
            // printf("ELF: Skipping header %s that doesn't need allocation\n", name);
            continue;
        }

        if (sh.flags & ELF_TYPE_EXECUTE) {
            // printf("ELF: Skipping header %s that is made of executable code (program headers already loaded)\n", name);
            continue;
        }

        memset((uint32_t *)sh.virtual_address, 0, sh.size);
        // Only copy bits if data region is non-zero (i.e. don't bother copying .bss)
        if (sh.type != ELF_SECTION_NOBITS) {
            memcpy((uint32_t *)sh.virtual_address, p_buffer + sh.offset, sh.size);
        }
    }

    // Finally, set the entry point to whatever it needs to be;
    *p_out_entry_point = (void *)p_header->entry_point;
    return true;
}