#include <iostream>
#include <string.h>

#define PORT    8080

#include "client.h"

int main(int argc, const char **argv)
{
    Client client(PORT);

    client.start();

    return 0;
}
