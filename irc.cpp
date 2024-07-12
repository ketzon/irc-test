#include "irc.hpp"

IrcServer::IrcServer(int p, const std::string &pw) : _port(p), _password(pw), _server_fd(-1) 
{
}

//setup telephone
void IrcServer::setupServerSocket() 
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //configure ligne telephonique
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(this->_port);
    address.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;

    //permet de reutiliser le telephone rapidement apres deco(socket)
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    //connecte le telephone a la ligne telephonique (adresse et port)
    if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address))) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //pret a recevoir des appel, manque plus que le config
    if (listen(_server_fd, 10) < 0) 
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    //configure telephone appel entrant
    pollfd server_pollfd;
    server_pollfd.fd = _server_fd;
    server_pollfd.events = POLLIN;
    _fds.push_back(server_pollfd);
}

void IrcServer::run() 
{
    setupServerSocket();
    std::cout << "Server running on port " << _port << " with password " << _password << std::endl;

    //garde le telephone en mode attente pour recevoir des events (call)
    while (true) 
    {
        int poll_count = poll(_fds.data(), _fds.size(), -1);
        if (poll_count < 0) 
        {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }

        //gere les event dentre pollin (gere les call client)
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

//repond a un appel telephonique
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

    pollfd client_pollfd;
    client_pollfd.fd = client_fd;
    client_pollfd.events = POLLIN;
    _fds.push_back(client_pollfd);

    std::cout << "New client connected" << std::endl;
}

//ecoute le message d'un client
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
        close(client_fd);

        // Remove from poll vector
        for (size_t i = 0; i < _fds.size(); ++i) 
        {
            if (_fds[i].fd == client_fd) 
            {
                _fds.erase(_fds.begin() + i);
                break;
            }
        }
    } 
    else 
    {
        buffer[bytes_read] = '\0';
        std::cout << "Received message: " << buffer << std::endl;
        // Echo message back to client
        write(client_fd, buffer, bytes_read);
    }
}

