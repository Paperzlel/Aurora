#pragma once

void clrscr();

/**
 * @brief Prints an inputted ASCII character to the current output stream. In the kernel, this often 
 * @param c The character to print
 */
void putc(char c);

void puts(const char *str);