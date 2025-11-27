#pragma once

#include <stdint.h>

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
