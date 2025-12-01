#pragma once

typedef enum {
    LEVEL_ERROR,
    LEVEL_WARNING,
    LEVEL_INFO,
    LEVEL_DEBUG
} LogLevel;

void log_message(LogLevel p_level, const char *p_module, const char *p_message, ...);


#define LOG_ERROR(m_msg, ...) log_message(LEVEL_ERROR, AUR_MODULE, m_msg, ##__VA_ARGS__)
#define LOG_WARNING(m_msg, ...) log_message(LEVEL_WARNING, AUR_MODULE, m_msg, ##__VA_ARGS__)
#define LOG_INFO(m_msg, ...) log_message(LEVEL_INFO, AUR_MODULE, m_msg, ##__VA_ARGS__)
#define LOG_DEBUG(m_msg, ...) log_message(LEVEL_DEBUG, AUR_MODULE, m_msg, ##__VA_ARGS__)
