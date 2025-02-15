#include "error.h"
#include "server.h"
#include <stdlib.h>

static struct error *error_head = NULL;

void add_error(int status_code, void (*handler)(int, struct request *))
{
    struct error *new_error = malloc(sizeof(struct error));
    new_error->status_code = status_code;
    new_error->handler = handler;
    new_error->next = error_head;
    error_head = new_error;
}

void send_error(int client_fd, int status_code, struct request *req)
{
    struct error *current = error_head;
    while (current)
    {
        if (current->status_code == status_code)
        {
            current->handler(client_fd, req);
            return;
        }
        current = current->next;
    }

    // Default error handler if no specific handler is found
    struct response res = {
        .version = "HTTP/1.1",
        .status_code = status_code,
        .status_text = "Error",
        .body = "An error occurred"
    };

    get_default_error_handler(client_fd, &res, status_code);
    send_response(client_fd, &res);
}

void get_default_error_handler(int client_fd, struct response *res, int code)
{

    switch (code)
    {
    case 400:
        res->status_text = "Bad Request";
        res->body = "400 Bad Request";
        break;
    case 401:
        res->status_text = "Unauthorized";
        res->body = "401 Unauthorized";
        break;
    case 403:
        res->status_text = "Forbidden";
        res->body = "403 Forbidden";
        break;
    case 404:
        res->status_text = "Not Found";
        res->body = "404 Not Foundsdfasdf";
        break;
    case 500:
        res->status_text = "Internal Server Error";
        res->body = "500 Internal Server Error";
        break;
    default:
        break;
    }
}

