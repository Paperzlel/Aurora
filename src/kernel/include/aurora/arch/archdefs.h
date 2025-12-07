#ifndef _AURORA_ARCHDEFS_H
#define _AURORA_ARCHDEFS_H

#include <aurora/kdefs.h>

#ifdef __I386__

// Struct defining the different registers that are pushed to the stack during an interrupt. Contains both 16 and 32 bit registers
struct __attribute__((packed)) Registers
{
    uint32_t ds;                    // Executing data segment
    union {
        uint32_t edi;                   // EDI register contents
        uint16_t di;                    // Lower 16 bits of EDI
    };
    union {
        uint32_t esi;                   // ESI register contents
        uint16_t si;                    // Lower 16 bits of ESI
    };
    union {
        uint32_t ebp;                   // EBP register contents
        uint16_t bp;                    // Lower 16 bits of EBP
    };

    uint32_t useless;               // ESP register from PUSHA, ignored as it's useless in this scenario.

    union {
        uint32_t ebx;                   // EBX register contents
        uint16_t bx;                    // Lower 16 bits of EBX
    };
    union {
        uint32_t edx;                   // EDX register contents
        uint16_t dx;                    // Lower 16 bits of EDX
    };
    union {
        uint32_t ecx;                   // ECX register contents
        uint16_t cx;                    // Lower 16 bits of ECX
    };
    union {
        uint32_t eax;                   // EAX register contents
        uint16_t ax;                    // Lower 16 bits of EAX
    };

    uint32_t interrupt;             // The interrupt that was called
    uint32_t error;                 // Error code (if supported, 0 if otherwise)

    union {
        uint32_t eip;                   // The instruction pointer at time of calling, is the exact instruction used
        uint16_t ip;                    // Lower 16 bits of EIP
    };

    uint32_t cs;                    // What code segment was being called
    
    union {
        uint32_t eflags;                // EFLAGS register, shows what different flags were set at the time.
        uint16_t flags;                 // Lower 16 bits of EFLAGS
    };
    union {
        uint32_t esp;                   // Stack pointer, only pushed when ring changes (i.e. calling an interrupt from ring 3 to ring 0) occur.
        uint16_t sp;                    // Lower 16 bits of ESP
    };

    uint32_t ss;                    // Stack segement, only pushed during ring changes, random value otherwise (like esp)
};

typedef bool (*InterruptHandler)(struct Registers *);

#else

struct __attribute__((packed)) Registers
{

};

typedef bool (*InterruptHandler)(struct Registers *);

#endif // __I386__
#endif // _AURORA_ARCHDEFS_H