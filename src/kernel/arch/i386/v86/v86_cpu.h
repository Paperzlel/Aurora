#pragma once

#include <stdint.h>

#define __cdecl __attribute__((cdecl))

void __cdecl v86_enter_v86(uint32_t ss, uint32_t esp, uint32_t cs, uint32_t eip);
void __cdecl v86_enter_v86_handler(uint32_t ss, uint32_t esp, uint32_t cs, uint32_t eip);

uint32_t __cdecl v86_cpu_get_esp();
uint32_t __cdecl v86_cpu_get_eip();

uint8_t __cdecl inb(uint16_t p_port);
uint16_t __cdecl inw(uint16_t p_port);

void __cdecl outb(uint16_t p_port, uint8_t p_value);
void __cdecl outw(uint16_t p_port, uint16_t p_value);
