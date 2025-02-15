
#ifndef SERVER_H
#define SERVER_H

#define MAX_HEADERS 32

struct header {
    char *name;
    char *value;
};

struct request {
    char *method;
    char *path;
    char *version;
    struct header headers[MAX_HEADERS];
    char *body;
};

struct response {
    char *version;
    int status_code;
    char *status_text;
    struct header headers[MAX_HEADERS];
    char *body;
};

void parse_request(char *buf, struct request *req);
void send_response(int client_fd, struct response *res);

void handle_client(int client_fd);

#endif