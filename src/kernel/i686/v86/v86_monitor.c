#include "v86_monitor.h"

#include "v86_cpu.h"

#include <i686/tss.h>

#include <stdint.h>
#include <memory.h>

Registers a_reg_state;

uint8_t getb(uint16_t p_seg, uint16_t p_ofs) {
    return *((uint8_t *)(p_seg << 4) + p_ofs);
}

uint16_t getw(uint16_t p_seg, uint16_t p_ofs) {
    return *((uint16_t *)(p_seg << 4) + p_ofs);
}

void pushw(Registers *p_regs, uint16_t p_word) {
    p_regs->esp -= 2;
    *(uint16_t *)((p_regs->ss << 4) + p_regs->esp) = p_word;
}

/**
 * @brief "Hack" to get into V86 mode.
 */
void v86_monitor_entry_handler(Registers *p_regs) {
    uint32_t data[] = { p_regs->eax, p_regs->ebx, p_regs->ecx, p_regs->edx };

    memcpy((void *)&a_reg_state, p_regs, sizeof(Registers));
    i686_tss_set_esp(v86_cpu_get_esp());
    i686_tss_set_eip(v86_cpu_get_eip());
    v86_enter_v86(data[0], data[1], data[2], data[3]);
}

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

void v86_load_task(void *p_task_start, void *p_task_run, void *p_task_end) {
    // Load task into code segment
    uint32_t size = p_task_end - p_task_start;
    memcpy((void *)(V86_CODE_SEGMENT << 4), p_task_start, size);

    // Enable exception handler hook and entry handler hook
    i686_isr_register_handler(13, v86_monitor_exception_handler);
    i686_isr_register_handler(0xfd, v86_monitor_entry_handler);

    // Call to ASM to set SS, SP, CS, IP
    // Boot into task
    v86_enter_v86_handler(V86_STACK_SEGMENT, 0x0000, V86_CODE_SEGMENT, p_task_run - p_task_start);
}
