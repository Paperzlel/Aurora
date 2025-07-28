#pragma once
#include <stdint.h>

#define NULL ((void *)0)

void clrscr();

/**
 * @brief Prints an inputted ASCII character to the current output stream.
 * @param c The character to print
 */
void putc(char c);

/**
 * @brief Prints a null-terminated ACII string to the current output stream. Non-null-terminated strings WILL read beyond their allocated
 * position and read invalid memory.
 * @param str The string to print into the console.
 */
void puts(const char *str);

/**
 * @brief Formats and prints a null-terminated string to the console. Format can include decimal/hexadecimal/octal numbers, characters, strings,
 * pointers and so on.
 * @param fmt The format string. To add a character from the varargs, add a % symbol followed by its type in command codes.
 * @param ... Variadic arguments, for specifying non-character inputs into the string.
 */
void printf(const char *fmt, ...);