#ifndef _AURORA_DEBUG_H
#define _AURORA_DEBUG_H

#ifndef AUR_MODULE
#error "Using <kernel/debug.h> requires a module tag to be used."
#define AUR_MODULE "(unknown)"
#endif

enum LogLevel
{
    LEVEL_FATAL,
    LEVEL_ERROR,
    LEVEL_WARNING,
    LEVEL_INFO,
    LEVEL_DEBUG
};

void log_message(enum LogLevel p_level, const char *p_module, const char *p_message, ...);

// Log a fatal message to the console
#define LOG_FATAL(m_msg, ...) log_message(LEVEL_FATAL, AUR_MODULE, m_msg, ##__VA_ARGS__)
// Log a non-fatal error message to the console
#define LOG_ERROR(m_msg, ...) log_message(LEVEL_ERROR, AUR_MODULE, m_msg, ##__VA_ARGS__)
// Log a warning to the console
#define LOG_WARNING(m_msg, ...) log_message(LEVEL_WARNING, AUR_MODULE, m_msg, ##__VA_ARGS__)
// Log generic information to the console
#define LOG_INFO(m_msg, ...) log_message(LEVEL_INFO, AUR_MODULE, m_msg, ##__VA_ARGS__)
// Log a debug message to the console. Disabled in release versions
#define LOG_DEBUG(m_msg, ...) log_message(LEVEL_DEBUG, AUR_MODULE, m_msg, ##__VA_ARGS__)

#endif // _AURORA_DEBUG_H