#pragma once

/**
 * @brief Disables interrupts and pauses forever. Only call if the OS requires to be panicked in such a way.
 */
void __attribute__((cdecl)) i686_panic();