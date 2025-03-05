#include "server.h"

void hello_world_handler(int client_fd, struct request *req)
{

    char* body = malloc(1024);
    
    for (int i = 0; i < req->params_count; i++)
    {
        sprintf(body + strlen(body), "%s: %s\n", req->params[i].name, req->params[i].value);
    }

    struct response res = {
        .version = "HTTP/1.1",
        .status_code = 200,
        .status_text = "OK",
        .body = body
    };
    send_response(client_fd, &res);
}

void test_post_handler(int client_fd, struct request *req)
{
    struct response res = {
        .version = "HTTP/1.1",
        .status_code = 200,
        .status_text = "OK",
        .body = "200 OK"
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

void setup_routes(struct server *server)
{
    add_route(server, GET, "/hello", hello_world_handler);
    add_route(server, POST, "/test", test_post_handler);
    add_route(server, GET, "/protected", protected_handler);
    // add_route(head, "/", home_handler);

    serve_static_files(server, "public");

    add_error(404, not_found_handler);
}
