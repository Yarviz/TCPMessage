#include "client.h"

Client::Client(int _port)
{
    port = _port;

    memset(buffer, 0, sizeof(buffer));

	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		std::cout << "Can't create socket!" << std::endl;
		exit(EXIT_FAILURE);
	}

    if ((my_socket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) == INVALID_SOCKET)
    {
        std::cout << "Can't create socket!" << std::endl;
        exit(EXIT_FAILURE);
    }

    address.sin_addr.s_addr = inet_addr("192.168.1.100");
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
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

    std::cout << "Client start!" << std::endl;

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

    closesocket(my_socket);

}
