#ifndef _STDLIB_H
#define _STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define __need_NULL
#define __need_size_t
#include <stddef.h>

int atexit(void (*func)(void));

int atoi(const char *ptr);

char *getenv(const char *name);

__attribute__((__noreturn__))
void abort();

void free(void *ptr);

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);

/**
 * @brief Obtains the absolute value of a given number.
 * @param j The inputted value
 * @returns The absolute value of j
 */
int abs(int j);

#ifdef __cplusplus
};
#endif

#endif