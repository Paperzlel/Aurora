#ifndef _STRING_H
#define _STRING_H

#define __need_NULL
#define __need_size_t
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Copies size bytes from the pointer src to dest.
 * @param dest The destination pointer
 * @param src The source pointer
 * @param size The number of bytes to copy
 * @returns The destination pointer.
 */
void *memcpy(void *dest, const void *src, int size);

/**
 * @brief Sets length bytes to value within dest.
 * @param dest The destination pointer
 * @param value The value to set the region to
 * @param length The number of bytes to set
 * @returns The destination pointer
 */
void *memset(void *dest, int value, int length);

/**
 * @brief Compares n bytes in the first pointer to the same n bytes in the second pointer and returns the difference between the two.
 * @param str1 The first pointer
 * @param str2 The second pointer
 * @param n The number of bytes to compare
 * @returns Zero if both pointers are the same, and the difference between byte str1[n] and byte str2[n] if not.
 */
int memcmp(const void *str1, const void *str2, int n);

/**
 * @brief Obtains the length of the inputted string. Assumes it is a null-terminated ASCII string.
 * @param s The input string.
 */
size_t strlen(const char *s);

/**
 * @brief Copies the contents of `s2` into the string represented by `s1`. Does not reallocate memory if the size of the two strings is different.
 * @param s1 The string to copy into
 * @param s2 The string to copy from
 * @returns The value of `s1` after being written to.
 */
char *strcpy(char *restrict s1, const char *restrict s2);

/**
 * @brief Concatenates two strings together, adding the string `s2` to the string `s1`. Does not reallocate memory if the string size is different.
 * @param s1 The string to concatenate onto
 * @param s2 The string to concatenate from
 * @returns The result of concatenating the two strings.
 */
char *strcat(char *restrict s1, const char *restrict s2);

/**
 * @brief Finds the first instance of the character `c` in the given string and returns the string up to that point. Returns `NULL` if no characters 
 * of the given type could be found.
 * @param s The string to look in
 * @param c The character to look for
 * @returns The string up to the first instance of the character being found.
 */
char *strchr(const char *s, int c);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _STRING_H