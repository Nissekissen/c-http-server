#ifndef SERVER_H
#define SERVER_H

#define MAX_HEADERS 32

#include "route.h"

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
};

struct response {
    char *version;
    int status_code;
    char *status_text;
    struct header headers[MAX_HEADERS];
    char *body;
};

struct thread_args {
    int client_fd;
    struct route *routes;
    struct error *errors;
};

void parse_request(char *buf, struct request *req);
void send_response(int client_fd, struct response *res);

void handle_client(void *arg);

void send_static_file(int client_fd, struct request *req);

// Serve static files from the "public" directory
void serve_static_files(struct route **head, char *path);

#endif