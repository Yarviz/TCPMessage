#include "client.h"

Client::Client(int _port)
{
    port = _port;

    buffer = new char[BUFFER_SIZE];

    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) raiseError(ERR_INIT);
    if ((my_socket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) == INVALID_SOCKET) raiseError(ERR_CREATION);

    address.sin_addr.s_addr = inet_addr("192.168.1.100");
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
}

Client::~Client()
{
    void freeResources();
}

void Client::freeResources()
{
    delete[] buffer;

    shutdown(my_socket, SD_SEND);
    closesocket(my_socket);
}

void Client::raiseError(int type)
{
    switch(type)
    {
        case ERR_INIT: perror("Error when initializing socket"); break;
        case ERR_CREATION: perror("Error when creating socket"); break;
        case ERR_OPTIONS: perror("Error when changing socket options"); break;
        case ERR_ADDRESS: perror("Error when creating address"); break;
        case ERR_CONNECT: perror("Error when connecting socket"); break;
        case ERR_SOCKET: perror("Error when polling sockets"); break;
    }

    freeResources();
    exit(EXIT_FAILURE);
}

void Client::clearMessageLine()
{
    std::string erase_line = '\r' + std::string(message.size(), ' ') + '\r';
    std::cout << erase_line << std::flush;
}

bool Client::sendMessage()
{
    std::cout << std::flush;
    send(my_socket, &message.c_str()[1], message.size() - 1, 0);
    message = ">";

    return true;
}

bool Client::handleKeyInput()
{
    if (!_kbhit()) return true;

    char c = getch();

    switch(c)
    {
        case -1:
            return true;
            break;

        case 8:
            if (message.size() > 1)
            {
                clearMessageLine();
                message.pop_back();
                std::cout << message;
            }
            break;

        case 13:
            if (message.size() == 1) return true;
            if (!sendMessage()) return false;
            break;

        case 27:
            return false;
            break;

        default:
            message += c;
            std::cout << c;
    }

    //std::cout << int(c);

    return true;
}

bool Client::readSocket()
{
    int read_size;

    if ((read_size = recv(my_socket, buffer, BUFFER_SIZE - 1, 0)) <= 0)
    {
        std::cout << std::endl << "Host disconnected." << std::endl;
        return false;
    }

    buffer[read_size] = '\0';
    clearMessageLine();
    std::cout << buffer << std::endl;
    std::cout << message << std::flush;

    return true;
}

void Client::start()
{
    if (connect(my_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cout << "Connection failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    int activity;
    timeval time_out;
    fd_set fds_read;

    message = " ";
    sendMessage();

    while(1)
    {
        FD_ZERO(&fds_read);
        FD_SET(my_socket, &fds_read);

        time_out.tv_sec = 0;
        time_out.tv_usec = 100;

        activity = select(my_socket + 1, &fds_read,  NULL, NULL, &time_out);

        if (activity < 0 && errno != EINTR) raiseError(ERR_SOCKET);

        if (FD_ISSET(my_socket, &fds_read))
        {
            if (!readSocket()) break;
        }

        if (!handleKeyInput()) break;
    }
}
