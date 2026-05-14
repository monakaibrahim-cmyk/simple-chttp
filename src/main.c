#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef USE_ENV_CONFIG
#include "common/parser/ENV/parser.h"
#elif defined(USE_CFG_CONFIG)
#include "common/parser/CFG/parser.h"
#endif

#include "Server/server.h"

#ifdef ENABLE_LOGGING
#include "common/Logs/logs.h"
#endif

#ifdef _WIN32
#include <windows.h>

HANDLE hStdin;
HANDLE hStdout;

DWORD oldMode;
CONSOLE_CURSOR_INFO oldCursor;

BOOL WINAPI handle_exit_signal(DWORD signal)
{
    switch (signal)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
        running = false;
        SetConsoleMode(hStdin, oldMode);
        SetConsoleCursorInfo(hStdout, &oldCursor);
        return TRUE;
    default:
        return FALSE;
    }
}
#else
#include <termios.h>
#include <unistd.h>

static struct termios oldt, newt;

void handle_exit_signal(int signal)
{
    (void)signal;
    running = false;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\033[?25h");
    fflush(stdout);
}
#endif



int main(int argc, char** argv)
{
#ifdef _WIN32
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SetConsoleCtrlHandler(handle_exit_signal, TRUE);
    GetConsoleMode(hStdin, &oldMode);

    DWORD newMode = oldMode;
    newMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    newMode |= ENABLE_PROCESSED_INPUT;
    SetConsoleMode(hStdin, newMode);
    FlushConsoleInputBuffer(hStdin);

    GetConsoleCursorInfo(hStdout, &oldCursor);

    CONSOLE_CURSOR_INFO ci = oldCursor;
    ci.bVisible = FALSE;
    ci.dwSize = 1;
    SetConsoleCursorInfo(hStdout, &ci);
#else
    signal(SIGINT, handle_exit_signal);
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("\033[?25l");
    fflush(stdout);

#endif
#ifdef ENABLE_LOGGING
    Logs.init("Logs", "app.log")
        ->set_level(debug)
        ->set_color(true);
#endif

#ifdef USE_CFG_CONFIG
    Config cfg;
    config_initialize(&cfg);

    if (!config_load(&cfg))
    {
        return EXIT_FAILURE;
    }

    const char* host = config_get(&cfg, "host");
    const char* port_str = config_get(&cfg, "port");
    int port = port_str ? atoi(port_str) : 8080;
    http_serve(host, port);
#elif defined(USE_ENV_CONFIG)
    env_load();
    const char* host = getenv("HOST");
    const char* port_str = getenv("PORT");
    int port = port_str ? atoi(port_str) : 8080;
    http_serve(host, port);
#endif

    return EXIT_SUCCESS;
}
