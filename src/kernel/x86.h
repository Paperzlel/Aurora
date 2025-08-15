#pragma once
#include <stdint.h>
#include <stdbool.h>

#define __cdecl __attribute__((cdecl))

void __cdecl x86_outb(uint16_t p_port, uint8_t p_data);

uint8_t __cdecl x86_inb(uint16_t p_port);