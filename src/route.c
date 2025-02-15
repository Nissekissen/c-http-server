#include "server.h"
#include "route.h"


void add_route(struct route **head, char *path, void (*handler)(int, struct request *))
{
    struct route *new_route = mallow(sizeof(struct route));
    new_route->path = path;
    new_route->handler = handler;
    new_route->next = *head;
    *head = new_route;
}

void handle_route(int client_fd, struct request *req, struct route *head)
{
    struct route *current = head;
    while (current) {
        if (strcmp(req->path, current->path) == 0) {
            current->handler(client_fd, req);
            return;
        }
        current = current->next;
    }
}