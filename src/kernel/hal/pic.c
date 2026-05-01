#include <aurora/arch/interrupts.h>
#include <aurora/kdefs.h>

#include <asm/io.h>

#define PIC_1 0x20
#define PIC_2 0xa0

#define PIC_1_COMMAND PIC_1
#define PIC_1_DATA	  (PIC_1 + 1)
#define PIC_2_COMMAND PIC_2
#define PIC_2_DATA	  (PIC_2 + 1)

#define ICW1_ENV_DATA	0x01
#define ICW1_CASCADE	0x02
#define ICW1_INTERVAL_4 0x04
#define ICW1_LEVEL		0x08
#define ICW1_INIT		0x10

#define ENV_8086	   0x01
#define ENV_AUTO	   0x02
#define ENV_BUF_SLAVE  0x04
#define ENV_BUF_MASTER 0x08
#define ENV_SFNM	   0x10

#define CASCADE_IRQ 0x02

#define PIC_EOI 0x20

// I don't know how to feel about this.
extern void __attribute__((cdecl)) i386_interrupt_handler(struct Registers *p_regs);

static uint32_t spurious_irqs_sent = 0;


static bool spurious_irq_handler(struct Registers *registers)
{
	uint8_t interrupt = 0;

	if (registers->interrupt == INT_IRQ_7)
	{
		outb(PIC_1_COMMAND, 0x0b);
		interrupt = inb(PIC_1_COMMAND);
		if (!interrupt)
		{
			spurious_irqs_sent++;
			return true;
		}
	}
	else if (registers->interrupt == INT_IRQ_15)
	{
		outb(PIC_2_COMMAND, 0x0b);
		interrupt = inb(PIC_2_COMMAND);
		if (!interrupt)
		{
			send_end_of_interrupt(INT_IRQ_0);
			return true;
		}
	}

	// Override and send the interrupt manually.
	registers->interrupt = interrupt;
	i386_interrupt_handler(registers);
	return true;
}

void send_end_of_interrupt(uint8_t p_irq)
{
	p_irq -= 0x20;
	if (p_irq > 8)
	{
		outb(PIC_2_COMMAND, PIC_EOI);
	}
	else
	{
		outb(PIC_1_COMMAND, PIC_EOI);
	}
}

void pic_initialize()
{
	// Remap all PICs to different values.
	// Should be done over ignoring them, as they will still give off interrupts regardless.
	outb(PIC_1_COMMAND, ICW1_INIT | ICW1_ENV_DATA);
	outb(PIC_2_COMMAND, ICW1_INIT | ICW1_ENV_DATA);

	outb(PIC_1_DATA, PIC_1); // Remap vectors to 0x20 and 0xa0 respectively
	outb(PIC_2_DATA, PIC_2);

	outb(PIC_1_DATA, 1 << CASCADE_IRQ);
	outb(PIC_2_DATA, 2);

	outb(PIC_1_DATA, ENV_8086);
	outb(PIC_2_DATA, ENV_8086);

	// Disable ALL interrupts, only allow handled ones through.
	outb(PIC_1_DATA, 0xff);
	outb(PIC_2_DATA, 0xff);

	// Set spurious IRQ handles.
	register_interrupt_handler(INT_IRQ_7, spurious_irq_handler);
	register_interrupt_handler(INT_IRQ_15, spurious_irq_handler);
}

void mask_irq(uint8_t p_irq)
{
	uint16_t port = PIC_1_DATA;
	uint8_t value = 0;
	if (p_irq >= 8)
	{
		port = PIC_2_DATA;
		p_irq -= 8;
	}

	// Preserve previous mask
	value = inb(port) | (1 << p_irq);
	outb(port, value);
}

void unmask_irq(uint8_t p_irq)
{
	p_irq -= 0x20;
	uint16_t port = PIC_1_DATA;
	uint8_t value = 0;
	if (p_irq >= 8)
	{
		port = PIC_2_DATA;
		p_irq -= 8;
	}

	// Preserve previous mask
	value = inb(port) & ~(1 << p_irq);
	outb(port, value);
}
