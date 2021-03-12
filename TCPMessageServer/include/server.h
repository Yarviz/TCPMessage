#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define MAX_CLIENTS     16
#define BUFFER_SIZE     4096

#define ERR_CREATION    0
#define ERR_OPTIONS     1
#define ERR_RECEIVE     2
#define ERR_BIND        3
#define ERR_LISTEN      4
#define ERR_SOCKET      5

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
        bool filterPacket();
        void parseEthernet(const uint8_t *buffer, int data_size);
        void parseIPheader(const uint8_t *buffer, int data_size);
        bool parseTCP(const uint8_t *buffer, int data_size);
        void parseUDP(const uint8_t *buffer, int data_size);
        void addNewClient();
        void readClient(int cl);

        uint16_t calculateChecksum(const uint8_t *buffer, uint16_t checksum, int data_size);

        struct clnt_sock
        {
            int num;
        };

        struct sockaddr_ll  raw_addr_ll;
        struct sockaddr     raw_addr;
        struct sockaddr_in  source, dest, master_addr;
        struct ethhdr      *eth_header;
        struct iphdr       *ip_header;
        struct tcphdr      *tcp_header;
        struct clnt_sock    clients[MAX_CLIENTS];
        fd_set              fds_read;

        int     port_number;
        unsigned int raw_addr_size;
        unsigned int master_addr_size;
        int     socket_raw, socket_master, new_socket;
        int     addres_in_len;
        uint8_t buffer[BUFFER_SIZE];
};

#endif // SERVER_H
