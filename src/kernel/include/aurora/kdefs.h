#ifndef _AURORA_KDEFS_H
#define _AURORA_KDEFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Macro to obtain the lesser of two values
 * @param x The first value
 * @param y The second value
 * @returns The lesser value of the two
 */
#define AMIN(x, y) ((x < y) ? x : y)

/**
 * @brief Macro to obtain the greater of two values
 * @param x The first value
 * @param y The second value
 * @returns The greater value of the two
 */
#define AMAX(x, y) ((x > y) ? x : y)


#define GCC_PRAGMA(m_content) _Pragma(#m_content)
#define GCC_PUSH_WARNING GCC_PRAGMA(GCC diagnostic push)
#define GCC_WARNING_IGNORE(m_warning) GCC_PRAGMA(GCC diagnostic ignored m_warning)
#define GCC_POP_WARNING GCC_PRAGMA(GCC diagnostic pop)

/* Assertion macro for whenever a calculation requires it */
#define STATIC_ASSERT(m_eval, m_error) _Static_assert(m_eval, m_error)

#endif // _AURORA_KDEFS_H