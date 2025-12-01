#ifndef _STDIO_H
#define _STDIO_H

#define __need_NULL
#define __need_size_t
#include <stddef.h>
#include <stdarg.h>

#define SEEK_SET 2

typedef struct { int unused; } FILE;

#ifdef __cplusplus
extern "C" {
#endif

extern FILE *stderr;

int fclose(FILE *stream);
int fflush(FILE *file);
FILE *fopen(const char *restrict filename, const char *restrict mode);

size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);

int fseek(FILE *stream, long int offset, int whence);
long int ftell(FILE *stream);

void setbuf(FILE *restrict stream, char *restrict buf);

int fprintf(FILE *stream, const char *fmt, ...);


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
 * @returns The number of characters transmitted (sent) to stdout, or a negative value if an encoding error occured.
 */
int printf(const char *restrict fmt, ...);

int sprintf(char *restrict s, const char *restrict format, ...);

int vfprintf(FILE *restrict stream, const char *restrict format, va_list args);

int vsprintf(char *restrict s, const char *restrict format, va_list args);

#ifdef __cplusplus
}
#endif

#endif // _STDIO_H