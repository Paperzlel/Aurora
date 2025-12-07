#pragma once

#include <stddef.h>

/**
 * @brief Returns the string after the first instance of c, or NULL if the string does not contain the character.
 * @param str The string to look through
 * @param c The character to look for
 * @returns NULL if the character doesn't exist in the string, or the string after the first instance of the character.
 */
const char *strchr(const char *str, char c);

/**
 * @brief Returns the length of the string, assuming the string itself is NULL-terminated.
 * @param str The string to find the length of
 * @returns The length of the string.
 */
unsigned strlen(const char *str);
