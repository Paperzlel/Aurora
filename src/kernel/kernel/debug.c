#define AUR_MODULE "debugger" /* Shouldn't really be defined here, but we DID make a standard for it, so... */
#include <aurora/debug.h>

#include <sys/time.h>
#include <asm/io.h>
#include <stdio.h>
#include <string.h>

#define MESSAGE_MAX 4096
#define TIME_SIZE 15    // Max 15 characters for the 

static timer_t timer;

void log_message(enum LogLevel p_level, const char *p_module, const char *p_message, ...)
{
    // Big stack variable, but should be enough
    char msg[MESSAGE_MAX];
    memset(msg, 0, MESSAGE_MAX);

    uint64_t time_ms = 0;
    uint8_t time_us = 0;
    if (timer_get_time(&timer))
    {
        time_ms = timer.time_ms;
        time_us = timer.time_us;
    }

    int num_size = sprintf(msg, "[%5lli.%03lli%01hhi] %s: ", time_ms / 1000, time_ms % 1000, time_us, p_module);

    // Format message
    va_list args;
    va_start(args, p_message);
    vsprintf(msg + num_size, p_message, args);
    va_end(args);

    size_t max_len = strlen(msg);
    msg[max_len] = '\n';
    msg[max_len + 1] = 0;

    // Don't bother with formatting
    puts(msg);

    if (p_level == LEVEL_FATAL)
    {
        panic();
    }
}
