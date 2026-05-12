#ifndef SERVER_H
#define SERVER_H

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

extern SOCKET server, client;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

extern int server, client;
#endif

extern volatile bool running;

#define MAX_SERVER_BUFFER_SIZE              1024
#define readonly                            const
#define ROOT_DIRECTORY                      "www"

#ifdef _WIN32
void server_handle_request(SOCKET client);
#else
void server_handle_request(int client);
#endif
void server_initialize(readonly char* host, readonly int port);

#endif // SERVER_H

