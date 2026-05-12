#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

// #define CONFIG_MODE_CFG
#define CONFIG_MODE_ENV

#if defined(CONFIG_MODE_CFG)
#include "common/parser/CFG/parser.h"
#elif defined(CONFIG_MODE_ENV)
#include "common/parser/ENV/parser.h"
#endif

#include "Server/server.h"
#include "common/Logs/logs.h"

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
    running = false;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\033[?25h");
    fflush(stdout);
    close(client);
    exit(0);
}
#endif



int main(int argc, char** argv)
{
    signal(SIGINT, handle_exit_signal);

#ifdef _WIN32
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SetConsoleCtrlHandler(signal_handler, TRUE);
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
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("\033[?25l");
    fflush(stdout);

#endif
    Logs.init("app.log")
        ->set_level(debug)
        ->set_color(true);

#if defined(CONFIG_MODE_CFG)
    Config cfg;
    config_initialize(&cfg);

    if (!config_load(&cfg))
    {
        return EXIT_FAILURE;
    }

    server_initialize(config_get(&cfg, "host"), atoi(config_get(&cfg, "port")));
#elif defined(CONFIG_MODE_ENV)
    env_load();
    server_initialize(getenv("HOST"), atoi(getenv("PORT")));
#endif
    

    return EXIT_SUCCESS;
}
