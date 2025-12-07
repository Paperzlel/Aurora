#pragma once

#include <stdbool.h>
#include <stdint.h>

// Structure defining a 32-bit ELF header.
struct __attribute__((packed)) ELF_Header 
{
    uint8_t magic[4];                               // Magic numbers, should equal 0x7F, 0x45, 0x4c, 0x46 (., 'E', 'L', 'F')
    uint8_t bit_size;                               // The number of bits this ELF file uses (1 = 32-bit, 2 = 64-bit)
    uint8_t endianess;                              // Whether the ELF file uses little-endian or big-endian values
    uint8_t elf_header_version;                     // The ELF header version (usually 1)
    uint8_t os_abi_target;                          // The Application Binary Interface target version (System V is a Unix-like default we wish to use)
    uint8_t os_abi_version;                         // The version of the ABI to use (ignored)
    uint8_t reserved[7];                            // Reserved bytes
    uint16_t object_type;                           // Type of object file (relocatable, executable, shared, core)
    uint16_t target_isa;                            // Target Instruction Set Architecture to use (generally x86)
    uint32_t original_elf;                          // ELF version itself (usually 1)
    uint32_t entry_point;                           // Pointer to the entry point of the file (in dynamic libraries, this is ignored)
    uint32_t prog_header_offset;                    // Pointer to the start of the program header entries
    uint32_t sect_header_offset;                    // Pointer to the start of the section header entries
    uint32_t flags;                                 // Flags, can be ignored on x86 as it's unused
    uint16_t main_header_size;                      // Size of this ELF header
    uint16_t program_header_entry_size;             // Size of an entry in the program header table
    uint16_t program_header_entry_count;            // Number of entries in the program header table
    uint16_t section_header_entry_size;             // Size of an entry in the section header table
    uint16_t section_header_entry_count;            // Number of entries in the section header table
    uint16_t section_header_entry_section_names;    // Section index to the section header string table (where the names of the sections are stored)
};

// Enumeration of bit sizes that are possible within ELF headers.
enum ELF_BitSize
{
    ELF_32BIT = 1,      // 32-bit header size
    ELF_64BIT = 2       // 64-bit header size
};

// Enumeration of endianness that is possible within ELF headers.
enum ELF_Endianess
{
    ELF_LE = 1,         // Little-endian
    ELF_BE = 2          // Big-endian
};

// Macro for detecting System V ABI in an ELF header
#define ELF_SYSTEMV 0x00

// Enumeration of different object file types that are possible within an ELF header
enum ELF_ObjectFileType
{
    ELF_RELOCATABLE = 0x01,             // ELF can be relocated
    ELF_EXECUTABLE = 0x02,              // ELF is an executable (and has an entrypoint)
    ELF_SHARED_OBJECT = 0x03,           // ELF is a shared object (like .so or .dll files)
    ELF_CORE_FILE = 0x04                // ELF is a "core" file (not sure what this means entirely)
};

// Enumeration that describes our supported ISAs in an ELF header
enum ELF_InstructionSetArchitecture
{
    ELF_X86 = 0x03,             // x86 architecture
    ELF_POWERPC32 = 0x14,       // PowerPC-32 architecture
    ELF_POWERPC64 = 0x15,       // PowerPC-64 architecture
    ELF_ARM32 = 0x28,           // ARM32 architecture
    ELF_IA64 = 0x32,            // IA-64 architecture
    ELF_AMD64 = 0x3e,           // AMD64 architecture
    ELF_ARM64 = 0xb7,           // ARM64 architecture
    ELF_RISCV = 0xf3            // RISC-V architecture
};

/**
 * @brief Checks the first four bytes of the file to see if it is an ELF file header
 * @param p_file The file to check
 * @returns True if it is an ELF file, and needs to be parsed as such, and false if not. False implies that the file is a .bin file or another format
 * without debugging information.
 */
bool elf_is_elf(uint8_t *p_file);

/**
 * @brief Checks if the formats specified in the ELF header table align with the expected formats.
 * @param p_file The ELF header to check
 * @returns True if the format is expected, and false if the format is invalid or not supported by the bootloader.
 */
bool elf_is_valid_format(struct ELF_Header *p_file);

/**
 * @brief Reads the ELF file and loads the sections into memory, provided they are valid.
 * @param p_header Pointer to the header file to cross-reference data with
 * @param p_buffer The loaded buffer that contains the ELF data
 * @param p_out_entry_point The entry point of the executable, if it exists
 * @returns True if successful, and false if something failed or was invalid.
 */
bool elf_read(struct ELF_Header *p_header, uint8_t *p_buffer, void **p_out_entry_point);
