#include "server.h"
#include "utils.h"

void parse_request(char *buf, struct request *req)
{
    printf("Parsing request\n");
    char *method_end = strchr(buf, ' ');
    if (!method_end)
    {
        printf("Invalid request: missing HTTP method\n");
        return;
    }
    *method_end = '\0';
    req->method = strdup(buf); // Allocate memory and copy

    char *path_start = method_end + 1;
    char *path_end = strchr(path_start, ' ');
    if (!path_end)
    {
        printf("Invalid request: missing URL path\n");
        return;
    }
    *path_end = '\0';

    // Extract URL path and parameters
    char *params_start = strchr(path_start, '?');
    if (params_start && *(params_start + 1) == '\0')
    {
        *params_start = '\0'; // No parameters if '?' is the last character
        params_start = NULL; // No parameters if '?' is the last character
    }
    req->params_count = 0;
    struct param *params = malloc(sizeof(struct param) * MAX_HEADERS);
    if (params_start)
    {
        *params_start = '\0';
        req->path = strdup(path_start); // Allocate memory and copy
        // The params should be split into name-value pairs and copied to params
        char *param = strtok(params_start + 1, "&");
        while (param)
        {
            char *equal = strchr(param, '=');
            if (equal)
            {
                *equal = '\0';
                printf("Setting param\n");
                params[req->params_count].name = malloc(strlen(param) + 1);
                params[req->params_count].value = malloc(strlen(equal + 1) + 1);
                urldecode2(params[req->params_count].name, param);
                urldecode2(params[req->params_count].value, equal + 1); // Allocate memory, copy, and decode
                req->params_count++;
                printf("Successfully set param\n");
            }
            param = strtok(NULL, "&");
        }

    }
    else
    {
        req->path = strdup(path_start); // Allocate memory and copy
        params[0].name = NULL;
        params[0].value = NULL;
    }

    printf("Trying to copy params to request\n");

    // Copy params to request->params
    memcpy(req->params, params, sizeof(struct param) * req->params_count);
    printf("Successfully copied params to request\n");

    char *version_start = path_end + 1;
    char *version_end = strstr(version_start, "\r\n");
    if (!version_end)
    {
        printf("Invalid request: missing version\n");
        return;
    }
    *version_end = '\0';
    req->version = strdup(version_start); // Allocate memory and copy

    struct header headers[MAX_HEADERS] = {0};

    // Headers
    char *line_start = version_end + 2;
    req->header_count = 0;
    while (line_start && req->header_count < MAX_HEADERS)
    {
        char *line_end = strstr(line_start, "\r\n");
        if (!line_end)
        {
            break;
        }
        *line_end = '\0';

        char *colon = strchr(line_start, ':');
        if (colon)
        {
            *colon = '\0';
            headers[req->header_count].name = strdup(line_start);
            headers[req->header_count].value = strdup(colon + 1);
            // Trim leading whitespace from value
            while (*headers[req->header_count].value == ' ')
                headers[req->header_count].value++;
            // printf("header: %s: %s\n", headers[req->header_count].name, headers[req->header_count].value);
            req->header_count++;
        }

        line_start = line_end + 2;
    }

    // copy headers to request using memcpy
    memcpy(req->headers, headers, sizeof(headers));

    printf("Done parsing!\n");
}

void format_headers(char *buf, struct response *response)
{
    // Format the headers into a string
    for (int i = 0; i < response->header_count; i++)
    {
        sprintf(buf + strlen(buf), "%s: %s\r\n", response->headers[i].name, response->headers[i].value);
        printf("Current header: %s: %s\n", response->headers[i].name, response->headers[i].value);
    }

    // Add an extra newline to indicate the end of the headers
    sprintf(buf + strlen(buf), "\r\n");
}

void add_required_headers(struct response *response, const char *content_type)
{
    // Add the Content-Length header
    char content_length[32];
    sprintf(content_length, "%lu", strlen(response->body));
    printf("Content-Length: %s\n", content_length);
    response->headers[response->header_count++] = (struct header){.name = "Content-Length", .value = strcpy(malloc(strlen(content_length) + 1), content_length)};

    // Add the Content-Type header
    response->headers[response->header_count++] = (struct header){.name = "Content-Type", .value = content_type};
}

void handle_client(void *thread_args)
{
    struct thread_args *args = (struct thread_args *) thread_args;
    struct server *server = args->server;
    int client_fd = args->client_fd;
    free(args);


    char buf[1024] = {0};
    char raw_request[1024] = {0};

    ssize_t bytes_received = recv(client_fd, buf, 1024 * sizeof(char), 0);
    if (bytes_received < 0)
    {
        if (errno == ENOTCONN)
        {
            printf("Client not connected\n");
        }
        else
        {
            perror("recv");
        }
        close(client_fd);
        return;
    }
    else if (bytes_received == 0)
    {
        printf("Client disconnected\n");
        close(client_fd);
        return;
    }

    
    strncpy(raw_request, buf, sizeof(raw_request) - 1);
    raw_request[sizeof(raw_request) - 1] = '\0'; // Ensure null-termination
    printf("Received request: %s\n", raw_request);

    struct request req = {0};
    parse_request(buf, &req);
    
    handle_route(client_fd, &req, server);

    close(client_fd);

    free(req.method);
    free(req.path);
    free(req.version);
}

void send_response(int client_fd, struct response *res)
{
    char buf[1024] = {0};
    sprintf(buf, "%s %d %s\r\n", res->version, res->status_code, res->status_text);

    add_required_headers(res, "text/plain");
    format_headers(buf, res);

    if (res->body)
    {
        sprintf(buf + strlen(buf), "%s", res->body);
    }

    // Print the response (debugging)
    printf("Response:\n%s\n", buf);

    send(client_fd, buf, strlen(buf), 0);
}

void send_static_file(int client_fd, struct request *req)
{

    char* full_path = malloc(strlen("public") + strlen(req->path) + 1);
    strcpy(full_path, "public");
    strcat(full_path, req->path);

    int fd = open(full_path, O_RDONLY);

    if (fd < 0)
    {
        perror("open");
        return;
    }

    // Send the HTTP response
    struct response res = {
        .version = "HTTP/1.1",
        .status_code = 200,
        .status_text = "OK",
        .body = 0};
    send_response(client_fd, &res);

    // Send the file
    char buf[1024];
    int n;
    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        send(client_fd, buf, n, 0);
    }

    close(fd);
    free(full_path);
}

void serve_static_files(struct server *server, char *path)
{
    // serve all files in the specified directory recursively
    DIR *dir;
    struct dirent *ent;
    struct stat st;

    if ((dir = opendir(path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            {
                continue;
            }

            char full_path[1024];
            sprintf(full_path, "%s/%s", path, ent->d_name);

            if (stat(full_path, &st) == 0)
            {
                if (S_ISREG(st.st_mode))
                {
                    char path_name[1024] = {0};
                    sprintf(path_name, "/%s", full_path + strlen("public/"));
                    printf("Adding route for %s\n", path_name);
                    add_route(server, path_name, send_static_file);
                }
                else if (S_ISDIR(st.st_mode))
                {
                    // Recursively serve files in subdirectory
                    serve_static_files(server, full_path);
                }
            }
        }
        closedir(dir);
    }
    else
    {
        perror("opendir");
    }
}

// server struct


int convert_port(int port)
{
    // Flip the port so 0x1234 -> 0x3412
    return ((port & 0x00FF) << 8) | ((port & 0xFF00) >> 8);
}

void init_server(struct server *server, int port)
{
    int converted_port = convert_port(port);
    server->port = converted_port;

    int s = socket(AF_INET, SOCK_STREAM, 0);


    if (s < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = converted_port,
        .sin_addr.s_addr = INADDR_ANY
    };

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(s, 10) < 0)
    {
        perror("listen");
        exit(1);
    }

    server->socket_fd = s;
}

void start_server(struct server *server)
{

    while (1) {

        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server->socket_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (*client_socket < 0) 
        {
            perror("accept");
            free(client_socket);
            continue;
        }

        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->server = server;
        args->client_fd = *client_socket;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, (void*) handle_client, (void *) args);
        pthread_detach(thread_id);

        free(client_socket);

    }
}