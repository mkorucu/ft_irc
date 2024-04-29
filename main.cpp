#include "Server.h"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "INVALID ARGUMENT" << std::endl;
        return -1;
    }

    Server server = Server(std::string(av[1]), std::string(av[2]));
	
    if (server.init() == IRC_OK)
        server.start();
}