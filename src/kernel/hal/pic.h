#pragma once

#include <kernel/kdefs.h>

typedef enum {
    IRQ_0 = 0x20,
    IRQ_1 = 0x21,
    IRQ_2 = 0x22,
    IRQ_3 = 0x23,
    IRQ_4 = 0x24,
    IRQ_5 = 0x25,
    IRQ_6 = 0x26,
    IRQ_7 = 0x27,
    IRQ_8 = 0x28,
    IRQ_9 = 0x29,
    IRQ_10 = 0x2a,
    IRQ_11 = 0x2b,
    IRQ_12 = 0x2c,
    IRQ_13 = 0x2d,
    IRQ_14 = 0x2e,
    IRQ_15 = 0x2f
} PIC_Irqs;

void pic_initialize();

void pic_mask_irq(uint8_t p_irq);

void pic_unmask_irq(uint8_t p_irq);