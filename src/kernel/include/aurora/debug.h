#ifndef _AURORA_DEBUG_H
#define _AURORA_DEBUG_H

#ifndef AUR_MODULE
#error "Using <kernel/debug.h> requires a module tag to be used."
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

#define LOG_FATAL(m_msg, ...) log_message(LEVEL_FATAL, AUR_MODULE, m_msg, ##__VA_ARGS__)
#define LOG_ERROR(m_msg, ...) log_message(LEVEL_ERROR, AUR_MODULE, m_msg, ##__VA_ARGS__)
#define LOG_WARNING(m_msg, ...) log_message(LEVEL_WARNING, AUR_MODULE, m_msg, ##__VA_ARGS__)
#define LOG_INFO(m_msg, ...) log_message(LEVEL_INFO, AUR_MODULE, m_msg, ##__VA_ARGS__)
#define LOG_DEBUG(m_msg, ...) log_message(LEVEL_DEBUG, AUR_MODULE, m_msg, ##__VA_ARGS__)

#endif // _AURORA_DEBUG_H