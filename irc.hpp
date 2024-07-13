#pragma once

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <string>
#include <poll.h>  

class User 
{
public:
    int fd;
    std::string nickname;
    std::string username;

    User(int fd) : fd(fd), nickname(""), username("") {}
};

class Channel 
{
public:
    std::string name;
    std::vector<User*> users;

    Channel() {}
    Channel(const std::string& name) : name(name) {}
};

class IrcServer 
{
public:
    IrcServer(int p, const std::string &pw);
    void run(void);

private:
    int _port;
    std::string _password;
    int _server_fd;
    std::vector<struct pollfd> _fds;
    std::map<int, User*> _users;
    std::map<std::string, Channel> _channels;

    void setupServerSocket();
    void acceptNewClient();
    void handleClientMessage(int client_fd);
    void handleCommand(int client_fd, const std::string &command);
    void removeClient(int client_fd);
};

