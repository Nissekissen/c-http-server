#ifndef ROUTE_H
#define ROUTE_H

struct request;
struct error;
struct server;

typedef enum {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS,
    HEAD
} http_method;

struct route {
    char *path;
    http_method method;
    
    void (*handler)(int, struct request *);

    struct route* next;
};

void add_route(struct server *server, http_method method, char *path, void (*handler)(int, struct request *));
void handle_route(int client_fd, struct request *req, struct server *server);
void setup_routes(struct server *server);

#endif