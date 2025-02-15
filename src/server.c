#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>

#include "server.h"
#include "utils.h"

void parse_request(char *buf, struct request *req)
{
    char *method_end = strchr(buf, ' ');
    if (!method_end)
    {
        printf("Invalid request\n");
        return;
    }
    *method_end = '\0';
    req->method = strdup(buf); // Allocate memory and copy

    char *path_start = method_end + 1;
    char *path_end = strchr(path_start, ' ');
    if (!path_end)
    {
        printf("Invalid request\n");
        return;
    }
    *path_end = '\0';
    req->path = strdup(path_start); // Allocate memory and copy

    char *version_start = path_end + 1;
    char *version_end = strstr(version_start, "\r\n");
    if (!version_end)
    {
        printf("Invalid request\n");
        return;
    }
    *version_end = '\0';
    req->version = strdup(version_start); // Allocate memory and copy

    struct header headers[MAX_HEADERS] = {0};

    // Headers
    char *line_start = version_end + 2;
    int i = 0;
    while (line_start && i < MAX_HEADERS)
    {
        char *line_end = strstr(line_start, "\r\n");
        if (!line_end)
        {
            break;
        }
        *line_end = '\0';

        char *colon = strchr(line_start, ':');
        if (colon)
        {
            *colon = '\0';
            headers[i].name = strdup(line_start);
            headers[i].value = strdup(colon + 1);
            // Trim leading whitespace from value
            while (*headers[i].value == ' ')
                headers[i].value++;
            // printf("header: %s: %s\n", headers[i].name, headers[i].value);
        }

        line_start = line_end + 2;
        i++;
    }

    // copy headers to request using memcpy
    memcpy(req->headers, headers, sizeof(headers));

}

void handle_client(void *arg)
{
    struct thread_args *args = (struct thread_args *)arg;
    int client_fd = args->client_fd;
    struct route *routes = args->routes;

    free(args);

    char buf[1024] = {0};
    char raw_request[1024] = {0};

    recv(client_fd, buf, sizeof(buf), 0);
    strcpy(raw_request, buf);

    struct request req = {0};
    parse_request(buf, &req);

    // printf("Parsed request: %s\n", req.version); // Debug statement

    handle_route(client_fd, &req, routes);

    close(client_fd);

    free(req.method);
    free(req.path);
    free(req.version);
}

void send_response(int client_fd, struct response *res)
{
    char buf[1024] = {0};
    sprintf(buf, "%s %d %s\r\n", res->version, res->status_code, res->status_text);

    for (int i = 0; i < MAX_HEADERS; i++)
    {
        if (res->headers[i].name)
        {
            sprintf(buf + strlen(buf), "%s: %s\r\n", res->headers[i].name, res->headers[i].value);
        }
    }

    sprintf(buf + strlen(buf), "\r\n");

    if (res->body)
    {
        sprintf(buf + strlen(buf), "%s", res->body);
    }

    send(client_fd, buf, strlen(buf), 0);
}

void send_static_file(int client_fd, struct request *req)
{

    char* full_path = malloc(strlen("public") + strlen(req->path) + 1);
    strcpy(full_path, "public");
    strcat(full_path, req->path);

    int fd = open(full_path, O_RDONLY);

    if (fd < 0)
    {
        perror("open");
        return;
    }

    // Send the HTTP response
    struct response res = {
        .version = "HTTP/1.1",
        .status_code = 200,
        .status_text = "OK",
        .body = 0};
    send_response(client_fd, &res);

    // Send the file
    char buf[1024];
    int n;
    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        send(client_fd, buf, n, 0);
    }

    close(fd);
    free(full_path);
}

void serve_static_files(struct route **head, char *path)
{
    // serve all files in the specified directory recursively
    DIR *dir;
    struct dirent *ent;
    struct stat st;

    if ((dir = opendir(path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            {
                continue;
            }

            char full_path[1024];
            sprintf(full_path, "%s/%s", path, ent->d_name);

            if (stat(full_path, &st) == 0)
            {
                if (S_ISREG(st.st_mode))
                {
                    char path_name[1024] = {0};
                    sprintf(path_name, "/%s", full_path + strlen("public/"));
                    printf("Adding route for %s\n", path_name);
                    add_route(head, path_name, send_static_file);
                }
                else if (S_ISDIR(st.st_mode))
                {
                    // Recursively serve files in subdirectory
                    serve_static_files(head, full_path);
                }
            }
        }
        closedir(dir);
    }
    else
    {
        perror("opendir");
    }
}