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
#define ERR_OPTIONS     1
#define ERR_ADDRESS     2
#define ERR_CONNECT     3
#define ERR_SOCKET      4


class Client
{
    public:
        Client(int _port);
        virtual ~Client();

        void start();

    private:
        void freeResources();
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
        int     oldf;
        bool    canonial;
};

#endif // CLIENT_H
