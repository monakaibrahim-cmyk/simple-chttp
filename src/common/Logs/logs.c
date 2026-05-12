#include "logs.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static struct
{
    FILE* file;
    severity_level level;
    bool enabled;
    mutex_t lock;
} State =
{
    NULL,
    debug,
    false,
#ifdef _WIN32
    NULL
#else
    PTHREAD_MUTEX_INITIALIZER
#endif
};

static readonly char* _lstr[] =
{
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

static readonly char* _cstr[] =
{
    WHITE,
    CYAN,
    GREEN,
    YELLOW,
    RED,
    MAGENTA
};

static Logger* init(readonly char* file)
{
#ifdef _WIN32
    if (State.lock == NULL)
    {
        State.lock = CreateMutex(NULL, FALSE, NULL);
    }
#endif

    mutex_lock(&State.lock);

    if (file)
    {
        State.file = fopen(file, "a");
    }

    mutex_unlock(&State.lock);

    return &Logs;
}

static Logger* set_level(severity_level level)
{
    State.level = level;
    
    return &Logs;
}

static Logger* set_color(bool flag)
{
    State.enabled = flag;
    
    return &Logs;
}

static void _indent_worker(FILE* out, int padding, readonly char* content, int width)
{
    int current = 0;
    readonly char* p = content;

    fprintf(out, "%*s", padding, "");
    
    current += padding;
    
    while (*p)
    {
        if (*p == '\n')
        {
            fprintf(out, "%*s", padding, "");
            
            current = padding;
            p++;
            continue;
        }

        fprintf(out, "%c", *p);
        
        current++;
        p++;

        if (current >= width && *p == ' ')
        {
            fprintf(out, "\n%*s", padding, "");

            current = padding;
            p++;
        }
    }

    fprintf(out, "\n");
}

static void _indent_file(FILE* out, int padding, readonly char* content)
{
    _indent_worker(out, 4, content, 80);
}

static void _indent_stdout(int padding, readonly char* content)
{
    _indent_worker(stdout, 4, content, 80);
}

static void _indent_stack_trace(FILE* out, int padding)
{
#ifdef _WIN32
    void* stack[32];
    HANDLE process = GetCurrentProcess();

    SymInitialize(process, NULL, TRUE);

    WORD frames = CaptureStackBackTrace(0, 32, stack, NULL);

    SYMBOL_INFO* symbol =
        (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256, 1);

    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    fprintf(out, "Stack Trace:\n");

    for (WORD i = 0; i < frames; i++)
    {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

        fprintf(out, "%*s%d: %s - 0x%0llX\n", padding, "", i, symbol->Name, symbol->Address);
    }

    free(symbol);
#else
    void* array[32];
    int size = backtrace(array, 32);
    char** symbols = backtrace_symbols(array, size);

    if (symbols != NULL)
    {
        fprintf(out, "Stack Trace:\n");

        for (int i = 0; i < size; i++)
        {
            fprintf(out, "%*s%s\n", padding, "", symbols[i]);
        }

        free(symbols);
    }
#endif
}

static void writec(severity_level level, const char* file, int line, const char* function, const char* fmt, ...)
{
    mutex_lock(&State.lock);

    if (level < State.level)
    {
        mutex_unlock(&State.lock);
        return;
    }

    char buffer[30];

    time_t now = time(NULL);

#ifdef _WIN32

    SYSTEMTIME st;
    GetLocalTime(&st);

    int milliseconds = st.wMilliseconds;

#else

    struct timeval tv;
    gettimeofday(&tv, NULL);

    int milliseconds = (int)(tv.tv_usec / 1000);

#endif

    struct tm tm_info;

#ifdef _WIN32
    localtime_s(&tm_info, &now);
#else
    localtime_r(&now, &tm_info);
#endif

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_info);

    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    if (State.enabled)
    {
        printf(
            "%s.%03d [%s%s%s] [%s%s%s:%s%d%s] [%s%s%s] %s\n",
            buffer,
            milliseconds,
            _cstr[level],
            _lstr[level],
            RESET,
            YELLOW,
            file,
            RESET,
            YELLOW,
            line,
            RESET,
            YELLOW,
            function,
            RESET,
            msg
        );
    }
    else
    {
        printf(
            "%s.%03d [%s] [%s:%d] [%s] %s\n", 
            buffer,
            milliseconds,
            _lstr[level],
            file,
            line,
            function,
            msg
        );
    }

    fflush(stdout);

    if (State.file != NULL)
    {
        fprintf(
            State.file,
            "%s.%03d [%s] [%s:%d] [%s] %s\n", 
            buffer,
            milliseconds,
            _lstr[level],
            file,
            line,
            function,
            msg
        );
        fflush(State.file);
    }

    mutex_unlock(&State.lock);
}

static void write_errors(severity_level level, const char* file, int line, const char* function, const char* fmt, ...)
{
    mutex_lock(&State.lock);

    if (level < warn)
    {
        mutex_unlock(&State.lock);
        return;
    }

    char buffer[30];
    struct timeval tv;
    gettimeofday(&tv, NULL);

    int milliseconds = lrint(tv.tv_usec / 1000.0);

    if (milliseconds >= 1000)
    {
        milliseconds -= 1000;
        tv.tv_sec++;
    }

    struct tm* t = localtime(&tv.tv_sec);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);

    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    const char* header = "\n================================================\n";
    const char* footer = "================================================\n\n";
    const char* horizontal = "------------------------------------------------";

    {
        printf("%s", header);

        if (State.enabled)
        {
            printf("Timestamp: %-10s.%03d\n", buffer, milliseconds);
            printf("Severity:  %s%-10s%s\n", _cstr[level], _lstr[level], RESET);
            printf("Thread:    %s%-10lu%s\n", GREEN, (unsigned long)pthread_self(), RESET);
            printf("Location:  %s%-10s%s:%s%d%s (%s)\n", YELLOW, file, RESET, YELLOW, line, RESET, function);
        }
        else
        {
            printf("Timestamp: %-10s.%03d\n", buffer, milliseconds);
            printf("Severity:  %-10s\n", _lstr[level]);
            printf("Thread:    %-10lu\n", (unsigned long)pthread_self());
            printf("Location:  %-10s:%d (%s)\n", file, line, function);
        }

        printf("Message:\n");
        indent(4, msg);
        printf("%s\n", horizontal);
        _indent_stack_trace(stdout, 4);
        printf("%s", footer);
        fflush(stdout);
    }

    if (State.file != NULL)
    {
        fprintf(State.file, "%s", header);
        fprintf(State.file, "Timestamp: %-10s.%03d\n", buffer, milliseconds);
        fprintf(State.file, "Severity:  %-10s\n", _lstr[level]);
        fprintf(State.file, "Thread:    %-10lu\n", (unsigned long)pthread_self());
        fprintf(State.file, "Location:  %-10s:%d (%s)\n", file, line, function);
        fprintf(State.file, "Message:\n");
        indent(State.file, 4, msg);

        fprintf(State.file, "%s\n", horizontal);
        fflush(State.file);

        _indent_stack_trace(State.file, 4);
        fprintf(State.file, "%s", footer);
        fflush(State.file);
    }

    mutex_unlock(&State.lock);
}

Logger Logs =
{
    .init = init,
    .set_level = set_level,
    .set_color = set_color,
    .write = writec,
    .write_err = write_errors
};

