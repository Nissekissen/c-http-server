#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "server.h"
#include "route.h"

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

void setup_routes(struct route **head)
{
    add_route(head, "/hello", hello_world_handler);
    // add_route(head, "/", home_handler);

    serve_static_files(head, "public");
}
