#ifndef _CTYPE_H
#define _CTYPE_H

/**
 * @brief Moves an ASCII character from uppercase to lowercase, if needed.
 * @param p_in The character to change case
 * @returns The ASCII character in lowercase format
 */
int tolower(int p_in);

/**
 * @brief Moves an ASCII character from lowercase to uppercase, if needed.
 * @param p_in The character to change case with
 * @returns The ASCII character in uppercase format
 */
int toupper(int p_in);

#endif // _CTYPE_H