#include "route.h"
#include "server.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#include "error.h"


void add_route(struct server *server, http_method method, char *path, void (*handler)(int, struct request *))
{
    struct route *new_route = malloc(sizeof(struct route));
    new_route->method = method;
    new_route->path = strdup(path); // Use strdup to copy the path
    new_route->handler = handler;
    new_route->next = server->routes;
    server->routes = new_route;

}

void handle_route(int client_fd, struct request *req, struct server *server)
{
    struct route *current = server->routes;
    while (current) {
        if (strcmp(req->path, current->path) == 0 && req->method == current->method) {
            current->handler(client_fd, req);
            return;
        }
        current = current->next;
    }
    
    send_error(client_fd, 404, req);
}