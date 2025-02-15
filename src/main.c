#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include "server.h"
#include "route.h"
#include "error.h"

#define PORT 0x901f // 8080


int main(int argc, char *argv[])
{
    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        AF_INET,
        PORT, // 8080
        0
    };

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    listen(s, 10);

    printf("Listening on port 8080\n");

    struct route *routes = NULL;
    setup_routes(&routes);

    while (1) {

        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(s, (struct sockaddr *)&client_addr, &addr_len);
        if (*client_socket < 0) 
        {
            perror("accept");
            free(client_socket);
            continue;
        }

        
        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->client_fd = *client_socket;
        args->routes = routes;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, args);
        pthread_detach(thread_id);

        free(client_socket); 

    }

    return 0;
}