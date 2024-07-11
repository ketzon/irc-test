#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>

class IrcServer {
public:
    IrcServer(int p, const std::string &pw);
    void run(void);

private:
    int _port;
    std::string _password;
    int _server_fd;
    std::vector<pollfd> _fds;

    void setupServerSocket();
    void acceptNewClient();
    void handleClientMessage(int client_fd);
};

