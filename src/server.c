#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#include "server.h"

void parse_request(char *buf, struct request *req)
{
    char *method_end = strchr(buf, ' ');
    if (!method_end) {
        printf("Invalid request\n");
        return;
    }
    *method_end = '\0';
    req->method = strdup(buf);

    char *path_start = method_end + 1;
    char *path_end = strchr(path_start, ' ');
    if (!path_end) {
        printf("Invalid request\n");
        return;
    }
    *path_end = '\0';
    req->path = strdup(path_start);

    char *version_start = path_end + 1;
    char *version_end = strstr(version_start, "\r\n");
    if (!version_end) {
        printf("Invalid request\n");
        return;
    }
    *version_end = '\0';
    req->version = strdup(version_start);

    struct header headers[MAX_HEADERS] = {0};

    // Headers
    char *line_start = version_end + 2;
    int i = 0;
    while (line_start && i < MAX_HEADERS) {
        char *line_end = strstr(line_start, "\r\n");
        if (!line_end) {
            break;
        }
        *line_end = '\0';

        char *colon = strchr(line_start, ':');
        if (colon) {
            *colon = '\0';
            headers[i].name = strdup(line_start);
            headers[i].value = strdup(colon + 1);

            // Trim leading whitespace from value
            while (*headers[i].value == ' ') headers[i].value++;
            printf("header: %s: %s\n", headers[i].name, headers[i].value);
        }

        line_start = line_end + 2;
        i++;
    }
}

void handle_client(int client_fd)
{
    char buf[1024] = {0};
    char raw_request[1024] = {0};

    recv(client_fd, buf, sizeof(buf), 0);
    strcpy(raw_request, buf);

    struct request req = {0};
    parse_request(buf, &req);    

    printf("Received:\n%s\n", raw_request);

    // Send a simple response
    struct response res = {
        "HTTP/1.1",
        200,
        "OK",
        {
            {"Content-Type", "text/plain"},
            {0}
        },
        "Hello, world!"
    };

    send_response(client_fd, &res);

    free(req.method);
    free(req.path);
    free(req.version);
}

void send_response(int client_fd, struct response *res)
{
    char buf[1024] = {0};
    sprintf(buf, "%s %d %s\r\n", res->version, res->status_code, res->status_text);

    for (int i = 0; i < MAX_HEADERS; i++) {
        if (res->headers[i].name) {
            sprintf(buf + strlen(buf), "%s: %s\r\n", res->headers[i].name, res->headers[i].value);
        }
    }

    sprintf(buf + strlen(buf), "\r\n");

    if (res->body) {
        sprintf(buf + strlen(buf), "%s", res->body);
    }

    send(client_fd, buf, strlen(buf), 0);
}