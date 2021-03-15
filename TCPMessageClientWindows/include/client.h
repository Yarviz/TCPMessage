#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>

#define BUFFER_SIZE 1024

#define ERR_INIT        0
#define ERR_CREATION    1
#define ERR_OPTIONS     2
#define ERR_ADDRESS     3
#define ERR_CONNECT     4
#define ERR_SOCKET      5

class Client
{
    public:
        Client(int _port);
        virtual ~Client();

        void start();

    private:
        void freeResources();
        void raiseError(int type);
        bool handleInput();
        void clearMessageLine();
        bool handleKeyInput();
        bool sendMessage();
        bool readSocket();

        sockaddr_in address;
        WSADATA     wsa;
        SOCKET      my_socket;

        int     addrlen;
        int     port;
        char   *buffer;
        std::string message;
};

#endif // CLIENT_H
