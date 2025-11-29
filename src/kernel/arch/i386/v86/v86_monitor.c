#include "v86_monitor.h"

#include "v86_cpu.h"

#include <arch/i386/isr.h>
#include <arch/i386/tss.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// Variable that all registers are saved to whenever the OS enters V86 and is loaded whenever the V86 program wants to exit
Registers a_reg_state;

uint8_t *a_arglist = 0;
int a_argc = 0;

/**
 * @brief Obtains the byte at the given segment:offset.
 * @param p_seg The segment to read from
 * @param p_ofs The offset to read from
 * @returns The byte found.
 */
uint8_t getb(uint16_t p_seg, uint16_t p_ofs) {
    return *(uint8_t *)((p_seg << 4) + p_ofs);
}

/**
 * @brief Obtains the word at the given segment:offset
 * @param p_seg The segment to read fromb
 * @param p_ofs The offset to read from
 * @returns The word found
 */
uint16_t getw(uint16_t p_seg, uint16_t p_ofs) {
    return *(uint16_t *)((p_seg << 4) + p_ofs);
}

/**
 * @brief Pushes a word to the stack
 * @param p_regs Pointer to all registers, including SS:SP
 * @param p_word The word to push to the stack
 */
void pushw(Registers *p_regs, uint16_t p_word) {
    p_regs->sp -= 2;
    *(uint16_t *)((p_regs->ss << 4) + p_regs->sp) = p_word;
}

uint16_t popw(Registers *p_regs) {
    uint16_t ret = getw(p_regs->ss, p_regs->sp);
    p_regs->sp += 2;
    return ret;
}

/**
 * @brief Entry handler to get into V86 mode from. Since we can't get the register values from C easily, we use an interrupt handler instead.
 * @param p_regs Pointer to all registers in their state from an interrupt
 */
bool v86_monitor_entry_handler(Registers *p_regs) {
    memcpy((void *)&a_reg_state, p_regs, sizeof(Registers));
    i386_tss_set_esp(v86_cpu_get_esp());
    i386_tss_set_eip(v86_cpu_get_eip());
    v86_enter_v86(p_regs->eax, p_regs->ebx, p_regs->ecx, p_regs->edx);
    return true;
}

/**
 * @brief Exception handler for V86 tasks. Will be called whenever a GPF occurs, and intends to emulate certain instructions for a V86 task.
 * @param p_regs Pointer to all registers from their state when an interrupt occurs
 */
bool v86_monitor_exception_handler(Registers *p_regs) {
    
    // Flee if not a VM86 fault
    if (!(p_regs->eflags & (1 << 17))) {
        return false;
    }

    uint8_t operation = getb(p_regs->cs, p_regs->ip);

    // [value] for any opcode refers to obtaining immediate values in code
    // registers may not be specific to the given value
    switch (operation) {
        case 0x9c: {    // pushf
            pushw(p_regs, p_regs->flags);
            ++p_regs->ip;
        } break;

        case 0x9d: {    // popf
            p_regs->flags = popw(p_regs);  // Instruction seems to mess up the stack, claims CS is C007 and is now invalid?
            ++p_regs->ip;
        } break;

        case 0xcd: {    // INT nn
            uint8_t int_id = getb(p_regs->cs, ++p_regs->ip); // Increment SP to move along to the next instruction
            // Copy old state over and return quickly
            if (int_id == 0xfe) {
                memcpy(p_regs, (void *)&a_reg_state, sizeof(Registers));
                break;
            }
            pushw(p_regs, p_regs->flags);      // Push FLAGS
            p_regs->eflags &= ~((1 << 8) | (1 << 9) | (1 << 18));   // Disable IF, TF, AC
            pushw(p_regs, p_regs->cs);          // Push CS
            pushw(p_regs, ++p_regs->ip);       // Push IP
            p_regs->cs = getw(0x0000, 4 * int_id + 2);  // Set to IVT offset
            p_regs->ip = getw(0x0000, 4 * int_id);     // Set to IVT offset
        } break;

        case 0xcf: {    // iret
            p_regs->ip = popw(p_regs);
            p_regs->cs = popw(p_regs);
            p_regs->flags = popw(p_regs);
        } break;
        
        case 0xe4: {    // in (al, [value])
            uint16_t port = getb(p_regs->cs, ++p_regs->ip);
            p_regs->ax = inb(port);
            ++p_regs->ip;
        } break;

        case 0xe5: {    // in (ax, [value])
            uint16_t port = getb(p_regs->cs, ++p_regs->ip);
            p_regs->ax = inw(port);
            ++p_regs->ip;
        } break;

        case 0xe6: {    // out ([value], al)
            uint16_t port = getb(p_regs->cs, ++p_regs->ip);
            outb(port, p_regs->ax);
            ++p_regs->ip;
        } break;

        case 0xe7: {    // out ([value], ax)
            uint16_t port = getb(p_regs->cs, ++p_regs->ip);
            outw(port, p_regs->ax);
            ++p_regs->ip;
        } break;

        case 0xec: {    // in (al, dx)
            p_regs->ax = inb(p_regs->dx);
            ++p_regs->ip;
        } break;

        case 0xed: {    // in (dx, ax)
            p_regs->ax = inw(p_regs->dx);
            ++p_regs->ip;
        } break;

        case 0xee: {    // out (dx, al)
            outb(p_regs->dx, p_regs->ax);
            ++p_regs->ip;
        } break;

        case 0xef: {    // out (dx, ax)
            outw(p_regs->dx, p_regs->ax);
            ++p_regs->ip;
        } break;

        case 0xFA: {    // cli
            p_regs->flags &= ~(1 << 9);    // Disable IF
            ++p_regs->ip;
        } break;

        case 0xFB: {    // sti
            p_regs->flags |= 1 << 9;   // Enable IF
            ++p_regs->ip;
        } break;

        default:
            printf("Unknown opcode %x\n", operation);
            return false;       // Unknown, throw an error
    }

    return true;
}

void v86_monitor_initialize() {
    // Do nothing for now
}

bool v86_run_task(void *p_task_start, void *p_task_end, uint8_t *p_args, int p_argc) {
    // Load task into code segment
    uint32_t size = p_task_end - p_task_start;
    memset((void *)(V86_CODE_SEGMENT << 4), 0, size);
    memcpy((void *)(V86_CODE_SEGMENT << 4), p_task_start, size);

    a_arglist = p_args;
    a_argc = p_argc;

    for (int i = 0; i < p_argc; i++) {
        *((uint8_t *)(V86_STACK_SEGMENT << 4) + i * 2) = p_args[i];
    }

    // Enable exception handler hook and entry handler hook
    i386_isr_register_handler(13, v86_monitor_exception_handler);
    i386_isr_register_handler(0xfd, v86_monitor_entry_handler);

    // Call to ASM to set SS, SP, CS, IP, then boot into task
    // Set offset depending on argument count (SP gets messed with otherwise)
    v86_enter_v86_handler(V86_STACK_SEGMENT, 0x0000 + (p_argc * 2) - 2, V86_CODE_SEGMENT, 0x0000);

    // Pass result into end pointer
    uint8_t *result = (uint8_t *)((V86_STACK_SEGMENT << 4) + 2);

    // Unregister handlers
    i386_isr_unregister_handler(0xfd);
    i386_isr_unregister_handler(13);

    return result;
}
