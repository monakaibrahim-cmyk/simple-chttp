#ifndef LOGS_H
#define LOGS_H

#include <stdbool.h>
#include <string.h>

#define BLACK                                      "\033[30m"
#define RED                                        "\033[31m"
#define GREEN                                      "\033[32m"
#define YELLOW                                     "\033[33m"
#define BLUE                                       "\033[34m"
#define MAGENTA                                    "\033[35m"
#define CYAN                                       "\033[36m"
#define WHITE                                      "\033[37m"
#define RESET                                      "\033[0m"

#define readonly                                   const
#define GET_INDENT_MACRO(_1, _2, _3, NAME, ...)    NAME
#define indent(...)                                GET_INDENT_MACRO(__VA_ARGS__, _indent_file, _indent_stdout)(__VA_ARGS__)

typedef enum
{
    trace,
    debug,
    info,
    warn,
    error,
    fatal
} severity_level;

typedef struct Logger Logger;

struct Logger
{
    Logger* (*init)(readonly char* file);
    Logger* (*set_level)(severity_level level);
    Logger* (*set_color)(bool flag);
    void (*write)(severity_level level, readonly char* file, int line, readonly char* function, readonly char* fmt, ...);
    void (*write_err)(severity_level level, readonly char* file, int line, readonly char* function, readonly char* fmt, ...);

};

static inline readonly char* trim_path(readonly char* path)
{
#ifdef _WIN32
    readonly char* p = strstr(path, "src\\");
#else
    readonly char* p = strstr(path, "src/");
#endif

    return p ? p : path;
}

extern Logger Logs;

#define LOG_TRACE(fmt, ...) \
    Logs.write(trace, trim_path(__FILE__), __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) \
    Logs.write(debug, trim_path(__FILE__), __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    Logs.write(info, trim_path(__FILE__), __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    Logs.write_err(warn, trim_path(__FILE__), __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    Logs.write_err(error, trim_path(__FILE__), __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
    
#define LOG_FATAL(fmt, ...) \
    Logs.write_err(fatal, trim_path(__FILE__), __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#endif // LOGS_H

