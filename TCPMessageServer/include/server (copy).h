#ifndef SERVER_H
#define SERVER_H

#define MAX_CLIENTS     16

#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <string.h>


class Server
{
    public:
        Server(int _port);
        virtual ~Server();

        void start();

    private:
        void addNewClient();
        void readClient(int cl);

        fd_set      fds_read;
        sockaddr_in address;

        int     master_socket, new_socket;
        int     max_sd, sd, active, addrlen;
        int     port;
        int     client_socket[MAX_CLIENTS];
        char    buffer[1024];
};

#endif // SERVER_H
