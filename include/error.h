#ifndef ERROR_H
#define ERROR_H

struct request;
struct response;

struct error {
    int status_code;
    void (*handler)(int, struct request *);

    struct error* next;
};

void get_default_error_handler(struct response *res, int code);
void add_error(int status_code, void (*handler)(int, struct request *));
void send_error(int client_fd, int status_code, struct request *req);

#endif