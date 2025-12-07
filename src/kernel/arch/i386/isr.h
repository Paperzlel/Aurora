#pragma once

// Shouldn't use this include, but since we need it upstream for the interrupt handlers, define some common code here :)
#include <aurora/arch/archdefs.h>

/**
 * @brief Initializes the ISR functions, or in other words loads all the needed data into the IDT and enables all the interrupts. Should save one or two for kernel
 * calls later on.
 */
void i386_isr_initialize();

/**
 * @brief Registers the given handler to manager any interrupts over the default handler. Generally, handlers will be used in brief contexts like a V86 monitor,
 * then pass interrupt control back to the default.
 * @param p_interrupt The interrupt to handle differently
 * @param p_handler The handler to use over the default
 * @returns True if the handler could be registered, and false if there already was a handler attributed to said interrupt.
 */
bool i386_isr_register_handler(int p_interrupt, InterruptHandler p_handler);

/**
 * @brief NULLs the given handler to this interrupt. 
 */
void i386_isr_unregister_handler(int p_interrupt);
