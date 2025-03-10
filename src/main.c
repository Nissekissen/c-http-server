#include "server.h"

int main(int argc, char *argv[])
{
    
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port == 0)
    {
        printf("Invalid port\n");
        return 1;
    }

    struct server server;
    init_server(&server, port);

    setup_routes(&server);

    start_server(&server);

    return 0;
}