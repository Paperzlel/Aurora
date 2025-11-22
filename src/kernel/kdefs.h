#pragma once

#include <stdint.h>
#include <stdbool.h>

#define GCC_PRAGMA(m_content) _Pragma(#m_content)
#define GCC_PUSH_WARNING GCC_PRAGMA(GCC diagnostic push)
#define GCC_WARNING_IGNORE(m_warning) GCC_PRAGMA(GCC diagnostic ignored m_warning)
#define GCC_POP_WARNING GCC_PRAGMA(GCC diagnostic pop)
