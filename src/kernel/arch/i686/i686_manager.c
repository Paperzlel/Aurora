#include "i686_manager.h"

#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "io.h"
#include "tss.h"
#include "pic.h"

void i686_initialize() {
    i686_gdt_initialize();
    i686_tss_initialize();
    i686_isr_initialize();
    i686_idt_initialize();
    i686_pic_initialize();
}

void i686_finalize() {
    return;     // Nothing
}