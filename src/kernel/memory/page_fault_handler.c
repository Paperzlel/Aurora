#include <aurora/arch/interrupts.h>
#include <aurora/memory.h>

#include <stdio.h>

enum PageFaultFlags {
	PAGE_PROTECTION_VIOLATION = 1 << 0,
	PAGE_WRITE_ACCESS = 1 << 1,
	PAGE_MAYBE_PRIVILEDGE_VIOLATION = 1 << 2,
	PAGE_RESERVED_WRITE = 1 << 3,
	PAGE_INSTRUCTION_FETCH = 1 << 4,
	PAGE_PROTECTION_KEY = 1 << 5,
	PAGE_SHADOW_STACK = 1 << 6,
	PAGE_SGX_VIOLATION = 1 << 15,
};

/**
 * @brief Exception handler for whenever we get a page fault. Page faults are handled depending on what error code was raised and where in memory it happened.
 * 
 * @param registers The registers that were passed onto the stack.
 * @return `false` every time. The system needs to panic about this since at the moment it's only running the kernel or a V86 task.
 */
bool page_fault_exception_handler(struct Registers *registers)
{
	// Get the message header out first.
	char *msg = "The kernel recieved a page fault:";
	uint32_t instruction = registers->eip;
	if (registers->error & PAGE_MAYBE_PRIVILEDGE_VIOLATION)
	{
		if (registers->eflags & EFLAGS_V86)
		{
			msg = "The Virtual 8086 emulator recieved a page fault:";
			instruction |= registers->cs << 4;
		}
		else
		{
			msg = "A user process recieved a page fault:";
		}
	}
	printf("%s\n", msg);

	// Check read/write info
	const char *r_or_w = registers->error & PAGE_WRITE_ACCESS ? "read" : "write to";
	
	// Check for if the entry was reserved, or if the address was invalid.
	char *was_write_invalid = "a non-present page entry.";
	uint32_t address = 0;
	if (registers->error & PAGE_PROTECTION_VIOLATION)
	{
		was_write_invalid = "a page and raised a protection fault.";
	}
	else
	{
		__asm__ volatile("mov %%cr2, %%eax\n\t"
			"mov %%eax, %0"
			: "=r"(address));
	}


	printf("Instruction 0x%x attempted to %s %s at address 0x%x\n", instruction, r_or_w, was_write_invalid, address);
	// TODO: Properly amend the page error.
	return false;
}

void register_page_fault_handler()
{
	register_interrupt_handler(0x0e, page_fault_exception_handler);
}
