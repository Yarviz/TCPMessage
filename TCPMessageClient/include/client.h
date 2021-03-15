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
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define BUFFER_SIZE    1024

#define ERR_CREATION    0
#define ERR_ADDRESS     1
#define ERR_CONNECT     2
#define ERR_BIND        3
#define ERR_LISTEN      4
#define ERR_SOCKET      5
#define ERR_INTERFACE   7
#define ERR_CLIENT      8
#define ERR_READ        9


class Client
{
    public:
        Client(int _port);
        virtual ~Client();

        void start();

    private:
        void raiseError(int type);
        void setCanonialMode(bool on_off);
        void clearMessageLine();
        bool handleInput();
        bool sendMessage();
        bool readSocket();

        fd_set      fds_read;
        sockaddr_in address;

        int     my_socket;
        int     max_sd, sd, active, addrlen;
        int     port;
        char   *buffer;
        std::string message;

        termios t_old, t_new;
        int oldf;
};

#endif // CLIENT_H
