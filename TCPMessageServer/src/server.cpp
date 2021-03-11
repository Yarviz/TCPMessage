#include "server.h"

Server::Server(int _port)
{
    port_number = _port;
    memset(buffer, 0, sizeof(buffer));

    if ((socket_raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0) raiseError(ERR_CREATION);

    std::cout << "Socket created succesfully!" << std::endl;
}

Server::~Server()
{
    //dtor
}

void Server::raiseError(int type)
{
    switch(type)
    {
        case ERR_CREATION: std::cout << "Error when creating socket."; break;
        case ERR_RECEIVE: std::cout << "Error when receiving packets."; break;
    }

    std::cout << std::endl;
    exit(EXIT_FAILURE);
}

uint16_t Server::calculateChecksum(const uint8_t *buffer, uint16_t checksum, int data_size)
{
    uint64_t sum = 0;
    uint16_t ret = 0;
    int bit = 0;

    sum += ntohl(ip_header->saddr);
    sum += ntohl(ip_header->daddr);
    sum += ip_header->protocol;
    sum += ntohs(ip_header->tot_len) - (ip_header->ihl * 4);

    while(data_size > 1)
    {
        sum += ntohs(*(uint16_t *)&buffer[bit]);
        bit += 2;
        data_size -= 2;
    }

    if (data_size == 1)
    {
        sum += *(uint8_t *)&buffer[bit] << 8;
    }

    sum -= checksum;

    while(sum > 0)
    {
        ret += sum & 0xffff;
        sum >>= 16;
    }

    return ~ret;
}

uint8_t Server::parseIPheader(const uint8_t *buffer, int data_size)
{
    ip_header = (struct iphdr*)buffer;

    source.sin_addr.s_addr = ip_header->saddr;
    dest.sin_addr.s_addr = ip_header->daddr;

    /*std::cout << "Address: ";
    std::cout << ((ip_header->saddr & 0x000000ff) >> 0) << ".";
    std::cout << ((ip_header->saddr & 0x0000ff00) >> 8) << ".";
    std::cout << ((ip_header->saddr & 0x00ff0000) >> 16) << ".";
    std::cout << ((ip_header->saddr & 0xff0000ff) >> 24) << std::endl;

    std::cout << ntohl(ip_header->saddr) << std::endl;*/

    std::cout << "Source IP: " << inet_ntoa(source.sin_addr) << std::endl;
    std::cout << "Destination IP: " << inet_ntoa(dest.sin_addr) << std::endl;
    std::cout << "IP header size: " << (int)ip_header->ihl * 4 << std::endl;

    return ip_header->protocol;
}

void Server::parseTCP(const uint8_t *buffer, int data_size)
{
    tcp_header = (struct tcphdr*)(buffer + ip_header->ihl * 4);

    std::cout << "Packet protocol: TCP" << std::endl;
    //std::cout << "Packet checksum: " << tcp_header-> << std::endl;
    std::cout << "Packet checksum: " << tcp_header->check << std::endl;
    std::cout << "Calculated checksum: " << calculateChecksum(buffer, tcp_header->check, data_size) << std::endl;
}

void Server::parseUDP(const uint8_t *buffer, int data_size)
{
    std::cout << "Packet protocol: UDP" << std::endl;
}

void Server::parsePacket(const uint8_t *buffer, int data_size)
{
    std::cout << std::endl;

    uint8_t protocol = parseIPheader(buffer, data_size);

    switch(protocol)
    {
        case PROT_TCP:
            parseTCP(buffer, data_size);
            break;

        case PROT_UDP:
            parseUDP(buffer, data_size);
            break;

        default:
            std::cout << "Packet protocol: UNDEFINED" << std::endl;
    }

    std::cout << std::endl;
}

void Server::start()
{
    int data_size;
    unsigned int addr_size = sizeof(address_in);
    int packets = 0;

    while(packets < 10)
    {
        data_size = recvfrom(socket_raw, buffer, BUFFER_SIZE, 0, &address_in, (socklen_t *)&addr_size);

        if (data_size < 0) raiseError(ERR_RECEIVE);

        parsePacket(buffer, data_size);
        ++packets;
    }

    close(socket_raw);
}
