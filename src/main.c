#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#include "server.h"

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

    printf("Listening on port %d\n", PORT);

    while (1) {
        int client_fd = accept(s, 0, 0);
        if (client_fd < 0) {
            perror("accept");
            return 1;
        }

        if (fork() == 0) {
            close(s);
            handle_client(client_fd);
            close(client_fd);
            exit(0);
        }

        close(client_fd);
    }

    return 0;
}