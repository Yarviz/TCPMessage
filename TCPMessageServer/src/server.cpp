#include "server.h"

Server::Server(int _port)
{
    port_number = _port;
    canonical_mode = false;

    buffer = new char[BUFFER_SIZE];

    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
    memset(clients, 0, sizeof(clients));
    memset(&packet, 0, sizeof(packet));

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

    //strncpy((char *)ifr.ifr_name, "wlp2s0", IFNAMSIZ);

    //if((ioctl(socket_raw, SIOCGIFINDEX, &ifr)) == -1) raiseError(ERR_INTERFACE);

    raw_addr_ll.sll_family = AF_PACKET;
    raw_addr_ll.sll_ifindex = ifr.ifr_ifindex;
    raw_addr_ll.sll_protocol = htons(ETH_P_IP);
}

Server::~Server()
{
    freeResources();
}

void Server::freeResources()
{
    delete[] buffer;

    setCanonicalMode(false);

    close(socket_raw);
    close(socket_master);
}

void Server::raiseError(int type)
{
    switch(type)
    {
        case ERR_CREATION: perror("Error when creating socket"); break;
        case ERR_OPTIONS: perror("Error when changing socket options"); break;
        case ERR_RECEIVE: perror("Error when receiving packets"); break;
        case ERR_BIND: perror("Error when binding socket"); break;
        case ERR_LISTEN: perror("Error when listening socket"); break;
        case ERR_SOCKET: perror("Error when polling sockets"); break;
        case ERR_INTERFACE: perror("Error when setting interface"); break;
        case ERR_CLIENT: perror("Error when adding client"); break;
        case ERR_READ: perror("Error when reading message"); return; break;
    }

    freeResources();
    exit(EXIT_FAILURE);
}

void Server::setCanonicalMode(bool on_off)
{

    if (on_off && !canonical_mode)
    {
        tcgetattr(fileno(stdin), &t_old);
        memcpy(&t_new, &t_old, sizeof(termios));
        t_new.c_lflag &= ~(ECHO | ICANON);
        t_new.c_cc[VTIME] = 0;
        t_new.c_cc[VMIN] = 1;
        tcsetattr(fileno(stdin), TCSANOW, &t_new);

        oldf = fcntl(fileno(stdin), F_GETFL, 0);
        fcntl(fileno(stdin), F_SETFL, oldf | O_NONBLOCK);
        canonical_mode = true;
    }
    else if (!on_off && canonical_mode)
    {
        tcsetattr(fileno(stdin), TCSANOW, &t_old);
        fcntl(fileno(stdin), F_SETFL, oldf);
        canonical_mode = false;
    }

}

uint16_t Server::calculateChecksum(const char *buffer, uint16_t checksum, int data_size)
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

void Server::parseEthernet()
{
    sprintf(packet.mac_address, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
            eth_header->h_source[0], eth_header->h_source[1], eth_header->h_source[2],
            eth_header->h_source[3], eth_header->h_source[4], eth_header->h_source[5]);
}

void Server::parseIPheader()
{
    source_addr.sin_addr.s_addr = ip_header->saddr;

    strcpy(packet.ip_address, inet_ntoa(source_addr.sin_addr));

    //std::cout << "Source IP: " << inet_ntoa(source.sin_addr) << std::endl;
    //std::cout << "Destination IP: " << inet_ntoa(dest.sin_addr) << std::endl;
    //std::cout << "IP header size: " << (int)ip_header->ihl * 4 << std::endl;
}

bool Server::parseTCP(const char *buffer, int data_size)
{
    tcp_header = (struct tcphdr*)(buffer + sizeof(struct ethhdr) + ip_header->ihl * 4);

    if (ntohs(tcp_header->th_dport) != port_number) return false;

    packet.cheksum = ntohs(tcp_header->check);
    packet.cheksum_calc = calculateChecksum(buffer, packet.cheksum, data_size);

    //std::cout << std::endl;
    //std::cout << "Packet protocol: TCP" << std::endl;
    //std::cout << "Packet checksum: " << tcp_header->check << std::endl;
    //std::cout << "Destination port: " << ntohs(tcp_header->th_dport) << std::endl;
    //std::cout << "Calculated checksum: " << calculateChecksum(buffer, tcp_header->check, data_size) << std::endl;

    return true;
}

void Server::parseUDP(const char *buffer, int data_size)
{
    std::cout << "Packet protocol: UDP" << std::endl;
}

void Server::parsePacket(const char *buffer, int data_size)
{
    eth_header = (struct ethhdr*)buffer;
    ip_header = (struct iphdr*)(buffer + sizeof(struct ethhdr));

    if (!filterPacket()) return;

    switch(ip_header->protocol)
    {
        case PROT_TCP:
            if (!parseTCP(buffer, data_size)) return;
            parseEthernet();
            parseIPheader();
            break;

        case PROT_UDP:
            parseUDP(buffer, data_size);
            break;

        //default:
        //    std::cout << "Packet protocol: UNDEFINED" << std::endl;
    }
}

void Server::printPacketInfo()
{
    std::cout << std::endl;
    std::cout << "Packet protocol     : TCP" << std::endl;
    std::cout << "Packet checksum     : " << packet.cheksum << std::endl;
    std::cout << "Calculated checksum : " << packet.cheksum_calc << std::endl;
    std::cout << "Packet IP           : " << packet.ip_address << std::endl;
    std::cout << "Packet MAC          : " << packet.mac_address << std::endl;
    std::cout << std::endl;
}

void Server::addNewClient()
{
    if ((new_socket = accept(socket_master, (struct sockaddr *)&master_addr, (socklen_t*)&master_addr_size)) < 0)
    {
        raiseError(ERR_CLIENT);
        return;
    }

    //if (source_addr.sin_addr.s_addr != master_addr.sin_addr.s_addr) return;

    std::cout << std::endl << "New connection: socket fd = " << new_socket << " ip = " << packet.ip_address;
    std::cout << " mac = " << packet.mac_address << std::endl;

    const char * message = "Welcome to Server";

    send(new_socket, message, strlen(message), 0);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].num == 0)
        {
            clients[i].num = new_socket;
            strcpy(clients[i].mac_address, packet.mac_address);
            break;
        }
    }
}

void Server::readClient(int cl)
{
    int read_size = read(clients[cl].num, buffer, BUFFER_SIZE - 1);

    if (read_size < 0)
    {
        raiseError(ERR_READ);
        return;
    }

    if (read_size == 0)
    {
        std::cout << clients[cl].mac_address << " Disconnected." << std::endl;

        close(clients[cl].num);
        clients[cl].num  = 0;
        return;
    }

    printPacketInfo();

    buffer[read_size] = '\0';
    char answer[20 + read_size];

    strcpy(answer, clients[cl].mac_address);
    strncpy(&answer[17], ": ", 3);
    strncpy(&answer[19], buffer, read_size + 1);

    //std::string answer = std::string(clients[cl].mac_address, 16) + ": " + std::string(buffer, read_size);
    std::cout << answer << std::endl;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].num > 0)
        {
            send(clients[i].num, answer, 20 + read_size, 0);
        }
    }
}

void Server::start()
{
    raw_addr_size = sizeof(raw_addr_ll);
    master_addr_size = sizeof(master_addr);
    int data_size, activity;
    int sd, max_sd;
    timeval time_out;

    if (bind(socket_master, (struct sockaddr *)&master_addr, master_addr_size) < 0) raiseError(ERR_BIND);
    if (bind(socket_raw, (struct sockaddr *)&raw_addr_ll, raw_addr_size) < 0) raiseError(ERR_BIND);

    if (listen(socket_master, 3) < 0) raiseError(ERR_LISTEN);

    setCanonicalMode(true);

    std::cout << "Server start: Listening on port " << port_number << std::endl;

    while(1)
    {
        FD_ZERO(&fds_read);
        FD_SET(socket_raw, &fds_read);
        FD_SET(socket_master, &fds_read);

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

        if (activity < 0 && errno != EINTR) raiseError(ERR_SOCKET);

        if (FD_ISSET(socket_raw, &fds_read))
        {
            data_size = recvfrom(socket_raw, buffer, BUFFER_SIZE - 1, 0, &raw_addr, (socklen_t *)&raw_addr_size);

            if (data_size < 0) raiseError(ERR_RECEIVE);
            parsePacket(buffer, data_size);
        }

        if (FD_ISSET(socket_master, &fds_read)) addNewClient();

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (FD_ISSET(clients[i].num , &fds_read)) readClient(i);
        }

        if (fgetc(stdin) == 27) break;
    }
}
