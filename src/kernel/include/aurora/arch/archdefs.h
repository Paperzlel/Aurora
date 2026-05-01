#ifndef _AURORA_ARCHDEFS_H
#define _AURORA_ARCHDEFS_H

#include <aurora/kdefs.h>

// Various flags available from the EFLAGS register.
enum EFLAGS_Options {
	EFLAGS_CARRY 		= 1 << 0,		// Set whenever a math operation has to carry a bit over.
	EFLAGS_PARITY 		= 1 << 2,		// Set if the number of bits set in the least significant byte of the last operation is even.
	EFLAGS_AUXILIARY 	= 1 << 4,		// Set if the last 4 significant bits in the previous operation required a carry.
	EFLAGS_ZERO 		= 1 << 6,		// Set if the last arithmetic operation result was 0.
	EFLAGS_SIGN 		= 1 << 7,		// Set if the last arithmetic operation was negative.
	EFLAGS_TRAP 		= 1 << 8,		// Set if the CPU permits single-step mode, used in many debuggers.
	EFLAGS_INTERRUPT 	= 1 << 9,		// Set if the CPU will respond to hardware interrupts (such as the PIT or floppy disk controller)
	EFLAGS_DIRECTION 	= 1 << 10,		// Set if the CPU will inrcement upwards on string opcodes. This should always be set.
	EFLAGS_OVERFLOW 	= 1 << 11,		// Set if the last arithmetic operation caused an integer overflow.
	EFLAGS_IOPL 		= 3 << 12,		// The current priviledge level of the CPU. Use an OR for this as it has a value of 0-3.
	EFLAGS_NESTED 		= 1 << 14,		// Set whenever a task that was just running is interrupted by another interrupt.
	EFLAGS_RESUME		= 1 << 16,		// Set if breakpoints can be ignored. Don't really bother with this.
	EFLAGS_V86			= 1 << 17,		// Set if the CPU is running in Virtual 8086 mode.
	EFLAGS_ALIGNMENT	= 1 << 18,		// Set if you want to ensure memory access is aligned properly (to 4-byte boundaries).
	EFLAGS_VIRTINT		= 1 << 19,		// Enabled if the VME flag in CR4 is set. If enabled, VME tasks will treat this flag as IF if the IOPL < 3.
	EFLAGS_VIRTPENDING 	= 1 << 20,		// Enabled if the VME flag in CR4 is set. If set, will generate a GPF to be handled.
	EFLAGS_CPUID		= 1 << 21,		// Set if the CPUID instruction exists. Pentium and higher only.
};



#ifdef __I386__

// Struct defining the different registers that are pushed to the stack during an interrupt. Contains both 16 and 32
// bit registers
struct __attribute__((packed)) Registers
{
	uint32_t ds; // Executing data segment
	union
	{
		uint32_t edi; // EDI register contents
		uint16_t di;  // Lower 16 bits of EDI
	};
	union
	{
		uint32_t esi; // ESI register contents
		uint16_t si;  // Lower 16 bits of ESI
	};
	union
	{
		uint32_t ebp; // EBP register contents
		uint16_t bp;  // Lower 16 bits of EBP
	};

	uint32_t useless; // ESP register from PUSHA, ignored as it's useless in this scenario.

	union
	{
		uint32_t ebx; // EBX register contents
		uint16_t bx;  // Lower 16 bits of EBX
	};
	union
	{
		uint32_t edx; // EDX register contents
		uint16_t dx;  // Lower 16 bits of EDX
	};
	union
	{
		uint32_t ecx; // ECX register contents
		uint16_t cx;  // Lower 16 bits of ECX
	};
	union
	{
		uint32_t eax; // EAX register contents
		uint16_t ax;  // Lower 16 bits of EAX
	};

	uint32_t interrupt; // The interrupt that was called
	uint32_t error;		// Error code (if supported, 0 if otherwise)

	union
	{
		uint32_t eip; // The instruction pointer at time of calling, is the exact instruction used
		uint16_t ip;  // Lower 16 bits of EIP
	};

	uint32_t cs; // What code segment was being called

	union
	{
		uint32_t eflags; // EFLAGS register, shows what different flags were set at the time.
		uint16_t flags;	 // Lower 16 bits of EFLAGS
	};
	union
	{
		uint32_t esp; // Stack pointer, only pushed when ring changes (i.e. calling an interrupt from ring 3 to ring 0)
					  // occur.
		uint16_t sp;  // Lower 16 bits of ESP
	};

	uint32_t ss; // Stack segement, only pushed during ring changes, random value otherwise (like esp)
};

typedef bool (*InterruptHandler)(struct Registers *);

#else

struct __attribute__((packed)) Registers
{
};

typedef bool (*InterruptHandler)(struct Registers *);

#endif // __I386__
#endif // _AURORA_ARCHDEFS_H