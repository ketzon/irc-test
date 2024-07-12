#include "irc.hpp"

int main(int ac, char **av) 
{
    if (ac != 3) 
        return std::cerr << "./ircserv <port> <password>\n", 1;
    int port = atoi(av[1]);
    std::string password = av[2];
    if (port <= 0 || port > 65535)
        return std::cerr << "Invalid port number" << std::endl, 1;
    IrcServer server(port, password);
    server.run();
    return 0;
}

