#include "server.h"

#ifdef ENABLE_LOGGING
#include "common/Logs/logs.h"
#endif

#ifdef _WIN32
SOCKET server, client;
#else
int server, client;
#endif

volatile bool running = true;

bool serve(int client, const char* filename)
{
    FILE* file = fopen(filename, "rb");

    if (!file)
    {
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* content = malloc(size + 1);
    content[size] = '\0';

    if (!content)
    {
        fclose(file);
        return false;
    }

    size_t read_size = fread(content, 1, size, file);

    if (read_size != (size_t)size)
    {
        fclose(file);
        free(content);
        return false;
    }

    fclose(file);

    char header[512];

    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: %ld\r\n\r\n", size);
#ifdef _WIN32
    send(client, header, strlen(header), 0);
    send(client, content, size, 0);
#else
    write(client, header, strlen(header));
    write(client, content, size);
#endif
    free(content);

    return true;
}

#ifdef _WIN32
void server_handle_request(SOCKET client)
#else
void server_handle_request(int client)
#endif
{
    char buffer[MAX_SERVER_BUFFER_SIZE];
    char method[10], path[100];
    char full_path[300];

#ifdef _WIN32
    int n = recv(client, buffer, MAX_SERVER_BUFFER_SIZE - 1, 0);
#else
    int n = read(client, buffer, MAX_SERVER_BUFFER_SIZE - 1);
#endif

    if (n <= 0)
    {
        return;
    }

    buffer[n] = '\0';

#ifdef ENABLE_LOGGING
    LOG_DEBUG("%s", buffer);
#endif

    sscanf(buffer, "%9s %99s", method, path);

#ifdef ENABLE_LOGGING
    LOG_HTTP("Request: %s %s", method, path);
#endif

    if (strcmp(path, "/") == 0)
    {
        strcpy(path, "/index.html");
    }

    if (strstr(path, "..") != NULL)
    {
        const char* response = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n403 - Forbidden";

#ifdef _WIN32
        send(client, response, strlen(response), 0);
#else
        write(client, response, strlen(response));
#endif
    }
    else
    {
        snprintf(full_path, sizeof(full_path), "%s%s", ROOT_DIRECTORY, path);

        if (!serve(client, full_path))
        {
            const char* response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n404 - File Not Found";

#ifdef _WIN32
            send(client, response, strlen(response), 0);
#else
            write(client, response, strlen(response));
#endif
        }
    }

#ifdef _WIN32
    closesocket(client);
#else
    close(client);
#endif
}

void server_initialize(const char* host, const int port)
{
#ifdef _WIN32
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        LOG_FATAL("WSA Startup failed!");
        exit(EXIT_FAILURE);
    }
#endif

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    int option = 1;
    int length = sizeof(address);

    server = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
    if (server == INVALID_SOCKET)
    {
        LOG_FATAL("Socket Creation Failed!");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, sizeof(option));
#else
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
#endif
    
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (host != NULL && host[0] != '\0')
    {
#ifdef _WIN32
        InetPtonA(AF_INET, host, &address.sin_addr);
#else
        inet_pton(AF_INET, host, &address.sin_addr);
#endif
    }
    else 
    {
        address.sin_addr.s_addr = INADDR_ANY;
    }

#ifdef _WIN32
    if (bind(server, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
#else
    if (bind(server, (struct sockaddr*)&address, sizeof(address)) < 0)
#endif
    {
        
#ifdef _WIN32
#ifdef ENABLE_LOGGING
        LOG_FATAL("Bind Failed! WSA Error=%d", socket_error());
#endif
        closesocket(server);
        WSACleanup();
#else
#ifdef ENABLE_LOGGING
        LOG_FATAL("Bind Failed! errno=%d", socket_error());
#endif
        close(server);
#endif
        exit(EXIT_FAILURE);
    }

#ifdef _WIN32
    if (listen(server, 10) == SOCKET_ERROR)
#else
    if (listen(server, 10) < 0)
#endif
    {
#ifdef _WIN32
#ifdef ENABLE_LOGGING
        LOG_FATAL("Listen Failed! WSA Error=%d", socket_error());
#endif
        closesocket(server);
        WSACleanup();
#else
#ifdef ENABLE_LOGGING
        LOG_FATAL("Listen Failed! errno=%d", socket_error());
#endif
        close(server);
#endif
        exit(EXIT_FAILURE);
    }

#ifdef ENABLE_LOGGING
    LOG_HTTP("HTTP Server Started on http://%s:%d", host, port);
#endif

    fd_set set;
    struct timeval timeout;

    while (running)
    {
        FD_ZERO(&set);
        FD_SET(server, &set);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int result = select(server + 1, &set, NULL, NULL, &timeout);
        
        if (result > 0 && FD_ISSET(server, &set))
        {
#ifdef _WIN32
            client = accept(server, (struct sockaddr*)&address, &length);

            if (client == INVALID_SOCKET)
#else
            client = accept(server, (struct sockaddr*)&address, (socklen_t*)&length);

            if (client < 0)
#endif
            {
                continue;
            }

            server_handle_request(client);
        }
    }

#ifdef _WIN32
    closesocket(server);
    WSACleanup();
#else
    close(server);
#endif
}
