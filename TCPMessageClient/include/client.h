#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>


class Client
{
    public:
        Client(int _port);
        virtual ~Client();

        void start();

    private:
        fd_set      fds_read;
        sockaddr_in address;

        int     my_socket;
        int     max_sd, sd, active, addrlen;
        int     port;
        char    buffer[1024];
};

#endif // CLIENT_H
