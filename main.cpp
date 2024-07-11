#include "irc.hpp"

int main(int ac, char **av) {
    if (ac != 3) {
        std::cerr << "./ircserv <port> <password>\n";
        return 1;
    }

    int port = atoi(av[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }

    std::string password = av[2];

    IrcServer server(port, password);
    server.run();

    return 0;
}

