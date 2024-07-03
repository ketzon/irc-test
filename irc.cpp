#include "irc.hpp"

Client::Client() { }
Client::~Client() { }

***************************ACCESSOR******************************

Client::getFd()
{
	return _fd;
}

Client::setFd(int fd)
{
	_fd = fd;
}

***************************************************************
