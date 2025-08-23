#pragma once

#include <stdint.h>
#include <stdbool.h>

// Struct defining the different registers that are pushed to the stack during an interrupt.
typedef struct {
    uint32_t ds;                    // Executing data segment
    uint32_t edi;                   // EDI register contents
    uint32_t esi;                   // ESI register contents
    uint32_t ebp;                   // EBP register contents
    uint32_t useless;               // ESP register from PUSHA, ignored as it's useless in this scenario.
    uint32_t ebx;                   // EBX register contents
    uint32_t edx;                   // EDX register contents
    uint32_t ecx;                   // ECX register contents
    uint32_t eax;                   // EAX register contents
    uint32_t interrupt;             // The interrupt that was called
    uint32_t error;                 // Error code (if supported, 0 if otherwise)
    uint32_t eip;                   // The instruction pointer at time of calling, is the exact instruction used
    uint32_t cs;                    // What code segment was being called
    uint32_t eflags;                // EFLAGS register, shows what different flags were set at the time.
    uint32_t esp;                   // Stack pointer, only pushed when ring changes (i.e. calling an interrupt from ring 3 to ring 0) occur.
    uint32_t ss;                    // Stack segement, only pushed during ring changes, random value otherwise (like esp)
} __attribute__((packed)) Registers;

typedef void (*InterruptHandler)(Registers *);

/**
 * @brief Initializes the ISR functions, or in other words loads all the needed data into the IDT and enables all the interrupts. Should save one or two for kernel
 * calls later on.
 */
void i686_isr_initialize();

/**
 * @brief Registers the given handler to manager any interrupts over the default handler. Generally, handlers will be used in brief contexts like a V86 monitor,
 * then pass interrupt control back to the default.
 * @param p_interrupt The interrupt to handle differently
 * @param p_handler The handler to use over the default
 * @returns True if the handler could be registered, and false if there already was a handler attributed to said interrupt.
 */
bool i686_isr_register_handler(int p_interrupt, InterruptHandler p_handler);

/**
 * @brief NULLs the given handler to this interrupt. 
 */
void i686_isr_unregister_handler(int p_interrupt);