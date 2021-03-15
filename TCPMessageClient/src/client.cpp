#include "client.h"

Client::Client(int _port)
{
    port = _port;

    buffer = new char[BUFFER_SIZE];

    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);

    if ((my_socket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) == 0) raiseError(ERR_CREATION);

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) raiseError(ERR_ADDRESS);
}

Client::~Client()
{
    delete[] buffer;
    setCanonialMode(false);

    shutdown(my_socket, SHUT_RDWR);
    close(my_socket);
}

void Client::setCanonialMode(bool on_off)
{

    if (on_off)
    {
        tcgetattr(fileno(stdin), &t_old);
        memcpy(&t_new, &t_old, sizeof(termios));
        t_new.c_lflag &= ~(ECHO | ICANON);
        t_new.c_cc[VTIME] = 0;
        t_new.c_cc[VMIN] = 1;
        tcsetattr(fileno(stdin), TCSANOW, &t_new);

        oldf = fcntl(fileno(stdin), F_GETFL, 0);
        fcntl(fileno(stdin), F_SETFL, oldf | O_NONBLOCK);
    }
    else
    {
        tcsetattr(fileno(stdin), TCSANOW, &t_old);
        fcntl(fileno(stdin), F_SETFL, oldf);
    }

}

void Client::raiseError(int type)
{
    switch(type)
    {
        case ERR_CREATION: perror("Error when creating socket"); break;
        case ERR_ADDRESS: perror("Error when creating address"); break;
        case ERR_CONNECT: perror("Error when connecting socket"); break;
        case ERR_BIND: perror("Error when binding socket"); break;
        case ERR_LISTEN: perror("Error when listening socket"); break;
        case ERR_SOCKET: perror("Error when polling socket"); break;
        case ERR_INTERFACE: perror("Error when setting interface"); break;
        case ERR_CLIENT: perror("Error when adding client"); break;
        case ERR_READ: perror("Error when reading message"); break;
    }

    exit(EXIT_FAILURE);
}

void Client::clearMessageLine()
{
    std::string erase_line = "\r" + std::string(" ", message.size()) + "\r";
    std::cout << erase_line;
    fflush(stdout);
}

bool Client::sendMessage()
{
    fflush(stdout);
    send(my_socket, &message.c_str()[1], message.size() - 1, 0);
    message = ">";

    if (!readSocket()) return false;
    return true;
}

bool Client::handleInput()
{
    char c = fgetc(stdin);

    switch(c)
    {
        case -1:
            return true;
            break;

        case 10:
            if (message.size() == 1) return true;
            if (!sendMessage()) return false;
            break;

        case 27:
            return false;
            break;

        case 127:
            if (message.size() > 1)
            {
                clearMessageLine();
                message.pop_back();
                std::cout << message;
            }
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

    if ((read_size = read(my_socket, buffer, BUFFER_SIZE - 1)) <= 0)
    {
        std::cout << std::endl << "Host disconnected." << std::endl;
        return false;
    }

    buffer[read_size] = '\0';
    clearMessageLine();
    std::cout << buffer << std::endl;
    std::cout << message;

    return true;
}

void Client::start()
{
    if (connect(my_socket, (struct sockaddr *)&address, sizeof(address)) < 0) raiseError(ERR_CONNECT);

    int activity;
    timeval time_out;
    fd_set fds_read;

    message = " ";
    sendMessage();

    setCanonialMode(true);

    while(1)
    {
        FD_ZERO(&fds_read);
        FD_SET(my_socket, &fds_read);

        time_out.tv_sec = 0;
        time_out.tv_usec = 100;

        activity = select(my_socket, &fds_read,  NULL, NULL, &time_out);

        if (activity < 0) raiseError(ERR_SOCKET);

        if (FD_ISSET(my_socket, &fds_read))
        {
            if (!readSocket()) break;
        }

        if (!handleInput()) break;
    }
}
