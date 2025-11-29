#include <kernel/memory.h>

#include <memory/memory_core.h>

void memfree(void *ptr) {
    kfree(ptr);
}

void *memalloc(size_t size) {
    kalloc(size);
}