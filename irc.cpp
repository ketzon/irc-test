#include "irc.hpp"

IrcServer::IrcServer(int p, const std::string &pw) : _port(p), _password(pw), _server_fd(-1) {}

void IrcServer::setupServerSocket() {
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(_server_fd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    pollfd server_pollfd;
    server_pollfd.fd = _server_fd;
    server_pollfd.events = POLLIN;
    _fds.push_back(server_pollfd);
}

void IrcServer::run() {
    setupServerSocket();
    std::cout << "Server running on port " << _port << " with password " << _password << std::endl;

    while (true) {
        int poll_count = poll(_fds.data(), _fds.size(), -1);
        if (poll_count < 0) {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _server_fd) {
                    acceptNewClient();
                } else {
                    handleClientMessage(_fds[i].fd);
                }
            }
        }
    }
}

void IrcServer::acceptNewClient() {
    sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int client_fd = accept(_server_fd, (struct sockaddr *)&client_address, &client_len);
    if (client_fd < 0) {
        perror("accept failed");
        return;
    }

    pollfd client_pollfd;
    client_pollfd.fd = client_fd;
    client_pollfd.events = POLLIN;
    _fds.push_back(client_pollfd);

    std::cout << "New client connected" << std::endl;
}

void IrcServer::handleClientMessage(int client_fd) {
    char buffer[1024];
    int bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            std::cout << "Client disconnected" << std::endl;
        } else {
            perror("read failed");
        }
        close(client_fd);

        // Remove from poll vector
        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].fd == client_fd) {
                _fds.erase(_fds.begin() + i);
                break;
            }
        }
    } else {
        buffer[bytes_read] = '\0';
        std::cout << "Received message: " << buffer << std::endl;
        // Echo message back to client
        write(client_fd, buffer, bytes_read);
    }
}

