#ifndef SERVER_H
#define SERVER_H

#define PORT    8080

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

class Server
{
    public:
        Server();
        virtual ~Server();

    protected:

    private:
};

#endif // SERVER_H