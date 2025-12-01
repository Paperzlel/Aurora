#pragma once

#include <stdint.h>

#ifdef __I386__

static inline void outb(uint16_t p_port, uint8_t p_value) {
    __asm__ volatile("outb %b0, %w1" : : "a"(p_value), "Nd"(p_port) : "memory");
}

static inline uint8_t inb(uint16_t p_port) {
    uint8_t ret;
    __asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(p_port) : "memory");
    return ret;
}

static inline void outw(uint16_t p_port, uint16_t p_value) {
    __asm__ volatile("outw %w0, %w1" : : "a"(p_value), "Nd"(p_port) : "memory");
}

static inline uint16_t inw(uint16_t p_port) {
    uint16_t ret;
    __asm__ volatile("inw %w1, %w0" : "=a"(ret) : "Nd"(p_port) : "memory");
    return ret;
}

static inline uint64_t rdmsr(uint32_t p_msr) {
    uint64_t msr_value;
    __asm__ volatile("rdmsr" : "=A" (msr_value) : "c" (p_msr));
    return msr_value;
}

static inline void wrmsr(uint32_t p_msr, uint64_t p_value)
{
    __asm__ volatile("wrmsr" : : "c" (p_msr), "A" (p_value));
}

static inline void panic()
{
    __asm__ volatile("cli");
    __asm__ volatile("hlt");
}

#endif