#pragma once

#include <stdint.h>

/**
 * @brief Moves an ASCII character from uppercase to lowercase, if needed.
 * @param p_in The character to change case
 * @returns The ASCII character in lowercase format
 */
char tolower(char p_in);

/**
 * @brief Moves an ASCII character from lowercase to uppercase, if needed.
 * @param p_in The character to change case with
 * @returns The ASCII character in uppercase format
 */
char toupper(char p_in);