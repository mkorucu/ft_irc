#include "Server.h"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "INVALID ARGUMENT" << std::endl;
        return -1;
    }
    try
    {
        Server server = Server(std::string(av[1]), std::string(av[2]));
        
        if (server.init() == IRC_OK)
            server.start();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return (0);
}
    