#ifndef ROUTE_H
#define ROUTE_H

struct request;
struct error;

struct route {
    char *path;
    void (*handler)(int, struct request *);

    struct route* next;
};

void add_route(struct route **head, char *path, void (*handler)(int, struct request *));
void handle_route(int client_fd, struct request *req, struct route *route_head, struct error *err_head);
void setup_routes(struct route **head);

#endif