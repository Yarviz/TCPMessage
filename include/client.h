#ifndef CLIENT_H
#define CLIENT_H

#define PORT    8080

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>


class Client
{
    public:
        Client();
        virtual ~Client();

    protected:

    private:
};

#endif // CLIENT_H
