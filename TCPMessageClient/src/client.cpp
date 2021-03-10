#include "client.h"

Client::Client(int _port)
{
    port = _port;

    memset(buffer, 0, sizeof(buffer));

    if ((my_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        std::cout << "Can't create socket!" << std::endl;
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0)
    {
        std::cout << "Invalid address!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

Client::~Client()
{
    //dtor
}

void Client::start()
{
    if (connect(my_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cout << "Connection failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    const char * message = "Hello Server!";

    while(1)
    {
        char ch = getchar();

        if (ch == 'q')
        {
            send(my_socket, message, strlen(message), 0);
            fflush(stdin);
        }
        else if (ch == 'w') break;
    }

}
