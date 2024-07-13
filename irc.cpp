#include "irc.hpp"
#include <sstream>
#include <algorithm>

IrcServer::IrcServer(int p, const std::string &pw) : _port(p), _password(pw), _server_fd(-1) 
{
}

void IrcServer::setupServerSocket() 
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(this->_port);
    address.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;

    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address))) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(_server_fd, 10) < 0) 
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    struct pollfd server_pollfd;
    server_pollfd.fd = _server_fd;
    server_pollfd.events = POLLIN;
    _fds.push_back(server_pollfd);
}

void IrcServer::run() 
{
    setupServerSocket();
    std::cout << "Server running on port " << _port << " with password " << _password << std::endl;

    while (true) 
    {
        int poll_count = poll(&_fds[0], _fds.size(), -1);
        if (poll_count < 0) 
        {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < _fds.size(); ++i) 
        {
            if (_fds[i].revents & POLLIN) 
            {
                if (_fds[i].fd == _server_fd) 
                {
                    acceptNewClient();
                } 
                else 
                {
                    handleClientMessage(_fds[i].fd);
                }
            }
        }
    }
}

void IrcServer::acceptNewClient() 
{
    sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int client_fd = accept(_server_fd, (struct sockaddr *)&client_address, &client_len);
    if (client_fd < 0) 
    {
        perror("accept failed");
        return;
    }

    struct pollfd client_pollfd;
    client_pollfd.fd = client_fd;
    client_pollfd.events = POLLIN;
    _fds.push_back(client_pollfd);

    User* new_user = new User(client_fd);
    _users[client_fd] = new_user;

    std::cout << "New client connected" << std::endl;
}

void IrcServer::handleClientMessage(int client_fd) 
{
    char buffer[1024];
    int bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read <= 0) 
    {
        if (bytes_read == 0) 
        {
            std::cout << "Client disconnected" << std::endl;
        } 
        else 
        {
            perror("read failed");
        }
        removeClient(client_fd);
    } 
    else 
    {
        buffer[bytes_read] = '\0';
        std::string command(buffer);
        handleCommand(client_fd, command);
    }
}

void IrcServer::handleCommand(int client_fd, const std::string &command) 
{
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "NICK") 
    {
        std::string nick;
        iss >> nick;
        _users[client_fd]->nickname = nick;
        std::string response = "Your nickname is now " + nick + "\n";
        write(client_fd, response.c_str(), response.size());
    } 
    else if (cmd == "JOIN") 
    {
        std::string channel_name;
        iss >> channel_name;

        if (_channels.find(channel_name) == _channels.end()) 
        {
            _channels[channel_name] = Channel(channel_name);
        }

        _channels[channel_name].users.push_back(_users[client_fd]);
        std::string response = "You joined channel " + channel_name + "\n";
        write(client_fd, response.c_str(), response.size());
    } 
    else 
    {
        std::string response = "Unknown command\n";
        write(client_fd, response.c_str(), response.size());
    }
}

void IrcServer::removeClient(int client_fd) 
{
    close(client_fd);

    for (size_t i = 0; i < _fds.size(); ++i) 
    {
        if (_fds[i].fd == client_fd) 
        {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }

    User* user = _users[client_fd];
    _users.erase(client_fd);
    delete user;

    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) 
    {
        Channel& channel = it->second;
        channel.users.erase(std::remove(channel.users.begin(), channel.users.end(), user), channel.users.end());
    }
}

