#include "v86_monitor.h"

#include "v86_cpu.h"

#include <arch/i686/isr.h>
#include <arch/i686/tss.h>

#include <stdint.h>
#include <memory.h>

// Variable that all registers are saved to whenever the OS enters V86 and is loaded whenever the V86 program wants to exit
Registers a_reg_state;

/**
 * @brief Obtains the byte at the given segment:offset.
 * @param p_seg The segment to read from
 * @param p_ofs The offset to read from
 * @returns The byte found.
 */
uint8_t getb(uint16_t p_seg, uint16_t p_ofs) {
    return *((uint8_t *)(p_seg << 4) + p_ofs);
}

/**
 * @brief Obtains the word at the given segment:offset
 * @param p_seg The segment to read from
 * @param p_ofs The offset to read from
 * @returns The word found
 */
uint16_t getw(uint16_t p_seg, uint16_t p_ofs) {
    return *((uint16_t *)(p_seg << 4) + p_ofs);
}

/**
 * @brief Pushes a word to the stack
 * @param p_regs Pointer to all registers, including SS:SP
 * @param p_word The word to push to the stack
 */
void pushw(Registers *p_regs, uint16_t p_word) {
    p_regs->esp -= 2;
    *(uint16_t *)((p_regs->ss << 4) + p_regs->esp) = p_word;
}

/**
 * @brief Entry handler to get into V86 mode from. Since we can't get the register values from C easily, we use an interrupt handler instead.
 * @param p_regs Pointer to all registers in their state from an interrupt
 */
void v86_monitor_entry_handler(Registers *p_regs) {
    memcpy((void *)&a_reg_state, p_regs, sizeof(Registers));
    i686_tss_set_esp(v86_cpu_get_esp());
    i686_tss_set_eip(v86_cpu_get_eip());
    v86_enter_v86(p_regs->eax, p_regs->ebx, p_regs->ecx, p_regs->edx);
}

/**
 * @brief Exception handler for V86 tasks. Will be called whenever a GPF occurs, and intends to emulate certain instructions for a V86 task.
 * @param p_regs Pointer to all registers from their state when an interrupt occurs
 */
void v86_monitor_exception_handler(Registers *p_regs) {
    uint8_t operation = getb(p_regs->cs, p_regs->eip);
    switch (operation) {
        case 0xCD: {    // INT nn
            uint8_t int_id = getb(p_regs->cs, ++p_regs->eip); // Increment SP to move along to the next instruction
            // Copy old state over and return quickly
            if (int_id == 0xfe) {
                memcpy(p_regs, (void *)&a_reg_state, sizeof(Registers));
                return;
            }
            pushw(p_regs, p_regs->eflags);      // Push EFLAGS
            p_regs->eflags &= ~((1 << 8) | (1 << 9) | (1 << 18));   // Disable IF, TF, AC
            pushw(p_regs, p_regs->cs);          // Push CS
            pushw(p_regs, ++p_regs->eip);       // Push IP
            p_regs->cs = getw(0x0000, 4 * int_id + 2);  // Set to IVT offset
            p_regs->eip = getw(0x0000, 4 * int_id);     // Set to IVT offset
        } break;

        default:
            break;  // Unknown, do nothing
    }
}

void v86_monitor_initialize() {
    // Do nothing for now
}

void v86_load_task(void *p_task_start, void *p_task_end) {
    // Load task into code segment
    uint32_t size = p_task_end - p_task_start;
    memcpy((void *)(V86_CODE_SEGMENT << 4), p_task_start, size);

    // Enable exception handler hook and entry handler hook
    i686_isr_register_handler(13, v86_monitor_exception_handler);
    i686_isr_register_handler(0xfd, v86_monitor_entry_handler);

    // Call to ASM to set SS, SP, CS, IP
    // Boot into task
    v86_enter_v86_handler(V86_STACK_SEGMENT, 0x0000, V86_CODE_SEGMENT, 0x0000);

    // Unregister handlers
    i686_isr_unregister_handler(0xfd);
    i686_isr_unregister_handler(13);
}
