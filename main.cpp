#include "irc.hpp"

int main(int ac, char **av)
{
	if (ac != 3) 
	{
		std::cout << "./ircserv <port> <password>\n";
		return 1;
	}
	(void)ac;
	(void)av;
}
