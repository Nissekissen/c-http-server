#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "server.h"
#include "route.h"
#include "error.h"

void hello_world_handler(int client_fd, struct request *req)
{
    struct response res = {
        .version = "HTTP/1.1",
        .status_code = 200,
        .status_text = "OK",
        .body = "Hello, World!"
    };
    send_response(client_fd, &res);
}

void protected_handler(int client_fd, struct request *req)
{
    send_error(client_fd, 401, req);
}

void not_found_handler(int client_fd, struct request *req)
{
    struct response res = {
        .version = "HTTP/1.1",
        .status_code = 404,
        .status_text = "Not Found",
        .body = "404 Not Found"
    };
    send_response(client_fd, &res);
}

void setup_routes(struct route **route_head)
{
    add_route(route_head, "/hello", hello_world_handler);
    add_route(route_head, "/protected", protected_handler);
    // add_route(head, "/", home_handler);

    serve_static_files(route_head, "public");

    add_error(404, not_found_handler);
}
