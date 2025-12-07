#ifndef _AURORA_INTERRUPTS_H
#define _AURORA_INTERRUPTS_H

#include "archdefs.h"

/**
 * Controls both IRQs and regular interrupts. Handlers for each routine can be connected and disconnected here, as well as the masking and unmasking
 * of IRQs.
 */

enum InterruptList
{
    INT_DIV_ZERO    = 0x00,
    INT_DEBUG       = 0x01,
    INT_NMI         = 0x02,
    INT_BREAKPOINT  = 0x03,
    INT_OVERFLOW    = 0x04,
    INT_BND_EXCPT   = 0x05,
    INT_INVLD_OP    = 0x06,
    INT_NO_MATH     = 0x07,
    INT_DBLFAULT    = 0x08,
    INT_CPSOVRN     = 0x09,
    INT_INVLD_TSS   = 0x0a,
    INT_SEG_NF      = 0x0b,
    INT_STK_SEG_FLT = 0x0c,
    INT_GPF         = 0x0d,
    INT_PAGE_FLT    = 0x0e,
    INT_RESVD       = 0x0f,
    INT_FP_ERROR    = 0x10,
    INT_ALGN_FLT    = 0x11,
    INT_MCHCHK      = 0x12,
    INT_SIMD_EXCP   = 0x13,
    INT_VIRT_EXCP   = 0x14,
    INT_CTRLPR_EXCP = 0x15,

    INT_IRQ_0       = 0x20,
    INT_IRQ_1       = 0x21,
    INT_IRQ_2       = 0x22,
    INT_IRQ_3       = 0x23,
    INT_IRQ_4       = 0x24,
    INT_IRQ_5       = 0x25,
    INT_IRQ_6       = 0x26,
    INT_IRQ_7       = 0x27,
    INT_IRQ_8       = 0x28,
    INT_IRQ_9       = 0x29,
    INT_IRQ_10      = 0x2a,
    INT_IRQ_11      = 0x2b,
    INT_IRQ_12      = 0x2c,
    INT_IRQ_13      = 0x2d,
    INT_IRQ_14      = 0x2e,
    INT_IRQ_15      = 0x2f,

    INT_SYSCALL     = 0xA0,
};

/**
 * @brief Registers an interrupt handler for the given interrupt. Any times the interrupt is raised by code, the output is redirected here instead of
 * causing a kernel panic.
 * @param p_isr The interrupt ID to bind to, as shown in `InterruptList`.
 * @param p_handle Function to handle the interrupt with. Must be of type `InterruptHandler`.
 * @return `true` if successful, and `false` if there already was an ISR there in the first place and we need to unregister it.
 */
bool register_interrupt_handler(uint8_t p_interrupt_id, InterruptHandler p_handler);

/**
 * @brief Masks one of the 15 IRQs raised by the 8259 PIC. These are raised by the hardware, and interrupts outside of these are not masked.
 * @param p_irq The IRQ to disable, for the time being.
 */
void mask_irq(uint8_t p_irq);

/**
 * @brief Unmasks one of the 15 IRQs raised by the 8259 PIC, effectively allowing our code to manage its events.
 * @param p_irq The IRQ to enable for the time being.
 */
void unmask_irq(uint8_t p_irq);

/**
 * @brief Sends an End of Interrupt signal to the PIC, essentially telling it that the OS is ready for the next load of interrupts to occur. 
 * @param p_irq The IRQ that has ended its interrupt.
 */
void send_end_of_interrupt(uint8_t p_irq);

#endif // _AURORA_INTERRUPTS_H