#include "route.h"
#include "server.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


void add_route(struct route **head, char *path, void (*handler)(int, struct request *))
{
    struct route *new_route = malloc(sizeof(struct route));
    new_route->path = strdup(path); // Use strdup to copy the path
    new_route->handler = handler;
    new_route->next = *head;
    *head = new_route;

    printf("Added route %s\n", new_route->path);
}

void handle_route(int client_fd, struct request *req, struct route *head)
{
    struct route *current = head;
    while (current) {
        printf("Comparing %s to %s\n", req->path, current->path);
        if (strcmp(req->path, current->path) == 0) {
            current->handler(client_fd, req);
            return;
        }
        current = current->next;
    }
}