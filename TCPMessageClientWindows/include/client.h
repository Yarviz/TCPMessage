#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <stdio.h>

class Client
{
    public:
        Client(int _port);
        virtual ~Client();

        void start();

    private:
        sockaddr_in address;
        WSADATA     wsa;
        SOCKET      my_socket;

        int     addrlen;
        int     port;
        char    buffer[1024];
};

#endif // CLIENT_H
