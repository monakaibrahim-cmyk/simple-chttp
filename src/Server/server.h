#ifndef SERVER_H
#define SERVER_H

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define _WIN32_WINNT                        0x0601

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define socket_error() WSAGetLastError()

extern SOCKET server, client;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#define socket_error() errno

extern int server, client;
#endif

extern volatile bool running;

#define MAX_SERVER_BUFFER_SIZE              1024
#define ROOT_DIRECTORY                      "www"

void http_serve(const char* host, const int port);

#endif // SERVER_H
