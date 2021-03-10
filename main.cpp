#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, const char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-server") == 0)
    {
        std::cout << "Start Server!" << std::endl;
    }
    else std::cout << "Start Client!" << std::endl;

    return 0;
}
