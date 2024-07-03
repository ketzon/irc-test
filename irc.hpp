#pragma once

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <iostream>
#include <iostream>


class Client
{
	public:
	Client();
	int getFd();
	void setFd(int fd);


	private:
	int _fd;
	std::string _ipAdd;
};
