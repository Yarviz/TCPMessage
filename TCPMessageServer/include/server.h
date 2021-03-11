#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define MAX_CLIENTS     16
#define BUFFER_SIZE     4096

#define ERR_CREATION    0
#define ERR_RECEIVE     1

#define PROT_TCP        6
#define PROT_UDP        17


class Server
{
    public:
        Server(int _port);
        virtual ~Server();

        void start();

    private:
        void raiseError(int type);
        void parsePacket(const uint8_t *buffer, int data_size);
        uint8_t parseIPheader(const uint8_t *buffer, int data_size);
        void parseTCP(const uint8_t *buffer, int data_size);
        void parseUDP(const uint8_t *buffer, int data_size);

        uint16_t calculateChecksum(const uint8_t *buffer, uint16_t checksum, int data_size);

        struct sockaddr     address_in;
        struct sockaddr_in  source, dest;
        struct iphdr       *ip_header;
        struct tcphdr      *tcp_header;

        int     port_number;
        int     client_socket[MAX_CLIENTS];
        int     socket_raw;
        int     addres_in_len;
        uint8_t buffer[BUFFER_SIZE];
};

#endif // SERVER_H
