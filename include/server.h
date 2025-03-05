#ifndef SERVER_H
#define SERVER_H

#define MAX_HEADERS 32

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

#include "route.h"
#include "error.h"

struct server {
    int port;
    int socket_fd;
    struct route *routes;
};

struct header {
    char *name;
    char *value;
};

struct param {
    char *name;
    char *value;
};

struct request {
    char *method;
    char *path;
    char *version;
    struct header headers[MAX_HEADERS];
    char *body;
    struct param params[MAX_HEADERS];
    int params_count;
    int header_count;
};

struct response {
    char *version;
    int status_code;
    char *status_text;
    struct header headers[MAX_HEADERS];
    char *body;
    int header_count;
};

struct thread_args {
    struct server *server;
    int client_fd;
};

void parse_request(char *buf, struct request *req);
void send_response(int client_fd, struct response *res);

void handle_client(void* thread_args);

void send_static_file(int client_fd, struct request *req);

// Serve static files from the "public" directory
void serve_static_files(struct server *server, char *path);

// server struct
void init_server(struct server *server, int port);
void start_server(struct server *server);

void format_headers(char *buf, struct response *response);
void add_required_headers(struct response *response, const char* content_type);

#endif