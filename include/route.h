#ifndef ROUTE_H
#define ROUTE_H

struct request;

struct route {
    char *path;
    void (*handler)(int, struct request *);

    struct route* next;
};

void add_route(struct route **head, char *path, void (*handler)(int, struct request *));
void handle_route(int client_fd, struct request *req, struct route *head);
void setup_routes(struct route **head);

#endif