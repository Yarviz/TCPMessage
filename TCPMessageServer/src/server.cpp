#include "server.h"

Server::Server(int _port)
{
    port_number = _port;
    memset(buffer, 0, sizeof(buffer));
    memset(clients, 0, sizeof(clients));

    if ((socket_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) raiseError(ERR_CREATION);
    if ((socket_master = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) raiseError(ERR_CREATION);

    int opt = 1;

    if (setsockopt(socket_master, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) raiseError(ERR_OPTIONS);
    if (setsockopt(socket_raw, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) raiseError(ERR_OPTIONS);

    bzero(&master_addr, sizeof(master_addr));

    master_addr.sin_family = AF_INET;
    master_addr.sin_addr.s_addr = INADDR_ANY;
    master_addr.sin_port = htons(port_number);

    ifreq ifr;

    bzero(&raw_addr, sizeof(raw_addr));
    bzero(&raw_addr_ll, sizeof(raw_addr_ll));
    bzero(&ifr, sizeof(ifr));

    strncpy((char *)ifr.ifr_name, "wlp2s0", IFNAMSIZ);
    //strncpy((char *)ifr.ifr_name, "lo", IFNAMSIZ);

    if((ioctl(socket_raw, SIOCGIFINDEX, &ifr)) == -1)
    {
        perror("Unable to find interface index");
        exit(-1);
    }

    raw_addr_ll.sll_family = AF_PACKET;
    raw_addr_ll.sll_ifindex = ifr.ifr_ifindex;
    raw_addr_ll.sll_protocol = htons(ETH_P_IP);

    //raw_addr.sa_family = AF_PACKET;
    //raw_addr.sin_addr.s_addr = INADDR_ANY;
    //raw_addr.sa_protocol = htons(ETH_P_IP);

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
        case ERR_CREATION: std::cout << "Error when creating socket"; break;
        case ERR_OPTIONS: std::cout << "Error when changing socket options"; break;
        case ERR_RECEIVE: std::cout << "Error when receiving packets"; break;
        case ERR_BIND: std::cout << "Error when binding socket"; break;
        case ERR_LISTEN: std::cout << "Error when listening socket"; break;
        case ERR_SOCKET: std::cout << "Error when polling sockets"; break;
    }

    std::cout << std::flush;
    perror(":");

    exit(EXIT_FAILURE);
}

uint16_t Server::calculateChecksum(const uint8_t *buffer, uint16_t checksum, int data_size)
{
    uint64_t sum = 0;
    uint16_t ret = 0;
    int bit = sizeof(struct ethhdr);
    data_size -= bit;

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

bool Server::filterPacket()
{
    if (ip_header->protocol == PROT_TCP) return true;

    return false;
}

void Server::parseEthernet(const uint8_t *buffer, int data_size)
{
    std::cout << "Destination MAC address: " << ether_ntoa((const struct ether_addr*)eth_header) << std::endl;
    printf("Source MAC address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n",
            eth_header->h_source[0], eth_header->h_source[1], eth_header->h_source[2],
            eth_header->h_source[3], eth_header->h_source[4], eth_header->h_source[5]);
    //std::cout << "Destination MAC address: " << ether_ntoa(&eth_header->h_dest) << std::endl;
}

void Server::parseIPheader(const uint8_t *buffer, int data_size)
{
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
}

bool Server::parseTCP(const uint8_t *buffer, int data_size)
{
    tcp_header = (struct tcphdr*)(buffer + sizeof(struct ethhdr) + ip_header->ihl * 4);

    //if (ntohl(tcp_header->th_dport) != port_number) return false;

    std::cout << std::endl;
    std::cout << "Packet protocol: TCP" << std::endl;
    std::cout << "Packet checksum: " << tcp_header->check << std::endl;
    std::cout << "Destination port: " << tcp_header->th_dport << std::endl;
    std::cout << "Calculated checksum: " << calculateChecksum(buffer, tcp_header->check, data_size) << std::endl;

    return true;
}

void Server::parseUDP(const uint8_t *buffer, int data_size)
{
    std::cout << "Packet protocol: UDP" << std::endl;
}

void Server::parsePacket(const uint8_t *buffer, int data_size)
{
    eth_header = (struct ethhdr*)buffer;
    ip_header = (struct iphdr*)(buffer + sizeof(struct ethhdr));

    if (!filterPacket()) return;

    switch(ip_header->protocol)
    {
        case PROT_TCP:
            if (!parseTCP(buffer, data_size)) return;
            parseEthernet(buffer, data_size);
            parseIPheader(buffer, data_size);
            break;

        case PROT_UDP:
            parseUDP(buffer, data_size);
            break;

        default:
            std::cout << "Packet protocol: UNDEFINED" << std::endl;
    }
}

void Server::addNewClient()
{
    if ((new_socket = accept(socket_master, (struct sockaddr *)&master_addr, (socklen_t*)&master_addr_size)) < 0)
    {
        std::cout << "Error when adding client!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "New connection: socket fd = " << new_socket << " ip = " << inet_ntoa(master_addr.sin_addr);
    std::cout << " port = " << ntohs(master_addr.sin_port) << " address family = " << master_addr.sin_family << std::endl;

    const char * message = "Welcome to Server";

    send(new_socket, message, strlen(message), 0) != strlen(message);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].num == 0)
        {
            clients[i].num = new_socket;
            break;
        }
    }
}

void Server::readClient(int cl)
{
    int read_size;

    if ((read_size = read(clients[cl].num, buffer, BUFFER_SIZE - 1)) == 0)
    {
        getpeername(clients[cl].num , (struct sockaddr*)&master_addr, (socklen_t*)&master_addr_size);
        std::cout << "Host disconnect: ip = " << inet_ntoa(master_addr.sin_addr) << " port = " << ntohs(master_addr.sin_port) << std::endl;

        close(clients[cl].num);
        clients[cl].num  = 0;
    }

    buffer[read_size] = '\0';
    std::cout << buffer << std::endl;
}

void Server::start()
{
    raw_addr_size = sizeof(raw_addr_ll);
    master_addr_size = sizeof(master_addr);
    int data_size, activity;
    int sd, max_sd;
    int packets = 0;
    timeval time_out;

    if (bind(socket_master, (struct sockaddr *)&master_addr, master_addr_size) < 0) raiseError(ERR_BIND);
    if (bind(socket_raw, (struct sockaddr *)&raw_addr_ll, raw_addr_size) < 0) raiseError(ERR_BIND);

    if (listen(socket_master, 3) < 0) raiseError(ERR_LISTEN);

    std::cout << "Server start: Listening on port " << port_number << std::endl;

    while(packets < 20)
    {
        FD_ZERO(&fds_read);
        FD_SET(socket_raw, &fds_read);
        //FD_SET(socket_master, &fds_read);

        max_sd = socket_master;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = clients[i].num;

            if (sd > 0) FD_SET(sd , &fds_read);
            if (sd > max_sd) max_sd = sd;
        }

        time_out.tv_sec = 1;
        time_out.tv_usec = 0;

        activity = select(max_sd + 1, &fds_read,  NULL, NULL, &time_out);

        if (activity < 0) raiseError(ERR_SOCKET);

        if (FD_ISSET(socket_raw, &fds_read))
        {
            data_size = recvfrom(socket_raw, buffer, BUFFER_SIZE - 1, 0, &raw_addr, (socklen_t *)&raw_addr_size);

            if (data_size < 0) raiseError(ERR_RECEIVE);
            parsePacket(buffer, data_size);
            ++packets;
        }
        if (FD_ISSET(socket_master, &fds_read)) addNewClient();

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (FD_ISSET(clients[i].num , &fds_read)) readClient(i);
        }
    }

    close(socket_raw);
    close(socket_master);
}
