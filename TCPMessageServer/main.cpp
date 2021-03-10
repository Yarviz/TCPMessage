#include <iostream>
#include <string.h>

#define PORT    8080

#include "server.h"

int main(int argc, const char **argv)
{
    //if (argc > 1 && strcmp(argv[1], "-server") == 0)
    Server server(PORT);

    server.start();

    return 0;
}
