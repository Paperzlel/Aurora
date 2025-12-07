#ifndef _AURORA_KDEFS_H
#define _AURORA_KDEFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define GCC_PRAGMA(m_content) _Pragma(#m_content)
#define GCC_PUSH_WARNING GCC_PRAGMA(GCC diagnostic push)
#define GCC_WARNING_IGNORE(m_warning) GCC_PRAGMA(GCC diagnostic ignored m_warning)
#define GCC_POP_WARNING GCC_PRAGMA(GCC diagnostic pop)

#endif // _AURORA_KDEFS_H