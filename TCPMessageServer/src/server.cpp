#include "server.h"

Server::Server(int _port)
{
    port = _port;
    memset(client_socket, 0, sizeof(client_socket));
    memset(buffer, 0, sizeof(buffer));

    if ((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        std::cout << "Can't create socket!" << std::endl;
        exit(EXIT_FAILURE);
    }

    int opt = 1;

    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        std::cout << "Can't change socket options!" << std::endl;
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    addrlen = sizeof(address);

    max_sd = 0;
    sd = 0;
    active = 0;
}

Server::~Server()
{
    //dtor
}

void Server::addNewClient()
{
    if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
    {
        std::cout << "Error when adding client!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "New connection: socket fd = " << new_socket << " ip = " << inet_ntoa(address.sin_addr) << " port = " << ntohs(address.sin_port) << std::endl;

    const char * message = "Welcome to Server";

    send(new_socket, message, strlen(message), 0) != strlen(message);

    for (int &socket : client_socket)
    {
        if(socket == 0)
        {
            socket = new_socket;
            break;
        }
    }
}

void Server::readClient(int cl)
{
    int read_size;

    if ((read_size = read(client_socket[cl], buffer, 1024)) == 0)
    {
        getpeername(client_socket[cl] , (struct sockaddr*)&address, (socklen_t*)&addrlen);
        std::cout << "Host disconnect: ip = " << inet_ntoa(address.sin_addr) << " port = " << ntohs(address.sin_port) << std::endl;

        close(client_socket[cl]);
        client_socket[cl] = 0;
    }

    std::cout << buffer << std::endl;
}

void Server::start()
{
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cout << "Can't bind socket!" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(master_socket, 3) < 0)
    {
        std::cout << "Error when listening!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server start: Listening on port " << port << std::endl;


    timeval time_out;

    while(1)
    {
        FD_ZERO(&fds_read);

        FD_SET(master_socket, &fds_read);
        max_sd = master_socket;

        for (int &socket : client_socket)
        {
            sd = socket;

            if (sd > 0) FD_SET( sd , &fds_read);
            if(sd > max_sd) max_sd = sd;
        }

        time_out.tv_sec = 1;
        time_out.tv_usec = 0;

        active = select(max_sd + 1, &fds_read,  NULL, NULL, &time_out);

        if (active < 0)
        {
            std::cout << "Socket error!" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(master_socket, &fds_read)) addNewClient();

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (FD_ISSET(client_socket[i] , &fds_read)) readClient(i);
        }

        //std::cout << "Waiting.." << std::endl;
    }
}
