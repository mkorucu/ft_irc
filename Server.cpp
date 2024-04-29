#include "Server.h"

Server::Server(std::string port_num, std::string pass)
{
    if (pass.length() <= 0)
    {
        std::cerr << "Password cannot be empty.\n";
        throw;
    }
    password = pass;
    try
    {
        port = std::stoi(port_num);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::cerr << "Portnum is not integer.\n";
    }
    if (port >= (1 << 16) || port < 0)
    {
        std::cerr << "Portnum is not between 2^16 ~ 0.\n";
    }
    std::cout << "Server object created.\n";
}

int Server::init()
{
    IRC_ERROR_CHECK(createSocket());
    IRC_ERROR_CHECK(setSocketOpt());
    IRC_ERROR_CHECK(setNonBlock());
    IRC_ERROR_CHECK(bindSocket());
    IRC_ERROR_CHECK(listenSocket());

    return 0;
}

void Server::start()
{
    FD_ZERO(&current_sockets);
    FD_ZERO(&ready_sockets);
    FD_SET(serverSocketFd, &current_sockets);
    max_socket = serverSocketFd;

    while(1)
    {
        ready_sockets = current_sockets;

        if(select(max_socket + 1, &ready_sockets, NULL, NULL, NULL) < 0){
            perror("select error");
            exit(EXIT_FAILURE);
        }

        for(int i = 0; i <= max_socket; i++)
        {
            if(FD_ISSET(i, &ready_sockets))
            {
                if(i == serverSocketFd)
                {
                    if((newClientFd = accept(serverSocketFd, (struct sockaddr *)&newClientSocketaddress, &newClientSocketAddressLen)) < 0)
                    {
                        std::cerr << "Accept Error.\n";
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        client_t client = {};
						client.socketFd = newClientFd;
                        myClients.push_back(client);
                        FD_SET(newClientFd, &current_sockets);
                        if(newClientFd > max_socket){
                            max_socket = newClientFd;
                        }
                        std::cout << "\033[1;30m" << "Client " << newClientFd << " connected to the server!" << "\033[0m" << std::endl;
                    }
                }
                else
                {
                    newClientFd = i;
                    ssize_t read_val = recv(newClientFd, buff, sizeof(buff), 0);
                    if (read_val <= 0)
                    {
                        if (read_val == 0)
						{
                            std::cout << "Client left the server, socket no: " << i << std::endl;
						}
                        else
						{
                            std::cerr << "recv error\n";
						}
						close(newClientFd);
						FD_CLR(newClientFd, &current_sockets);
						std::vector<client_t>::iterator index = findClient(newClientFd);
						if (index != myClients.end())
							myClients.erase(index);
                    }
                    else if (read_val == 1) // '\n'
                        continue;
                    else
                    {
						buff[read_val - 1] = '\0';
						std::cout << "\033[32m" << "Client " << newClientFd << ": " << "\033[0m" << buff << std::endl;
						send(newClientFd, "\033[32mMessage is successfully received.\033[0m\n", 44, 0);
						parseClient(i);
                    }
                }
            }
        }
        for(size_t i = 0; i < myChannels.size(); i++)
        {
            for(size_t j = 0; j < myChannels.at(i).operator_array.size(); j++)
            {
                std::cout << myChannels.at(i).name << " operator is: " << myChannels.at(i).operator_array.at(j).nickname << std::endl;
            }
        }
    }
}

void Server::parseClient(int i)
{
	(void) i;


    std::istringstream iss(buff);
	std::string token;
	std::vector<std::string> tokens;
	while (std::getline(iss, token, ' '))
	{
		if (!token.empty())
    		tokens.push_back(token);
    }
	std::vector<std::string>::iterator it = tokens.begin();
	std::vector<client_t>::iterator client_it = findClient(newClientFd);
	if ((*it) == "PASS")
	{
		std::cout << "Pass is envoked" << std::endl; 
		// TO DO: bunları pass_check(client_it) içine koy
		if ((it + 1) != tokens.end() && tokens.size() == 2)
		{
			if (client_it->is_auth == true)
			{
				sendToClient(ERR_ALREADYREGISTERED(nickname));
			}
			else if (*(it + 1) != this->password)
			{
				sendToClient("Password incorrect.");
			}
			else
			{
				sendToClient("Password correct.");
				client_it->is_auth = true;
			}

		}
		else
		{
			sendToClient("Wrong formatting. use PASS <password> ");
		}
		
	}
	else if ((*it) == "NICK")
	{
		std::cout << "NICK is envoked" << std::endl;
		
		if ((it + 1) != tokens.end() && tokens.size() == 2)
		{
			if (client_it->is_auth == false)
			{
				sendToClient("Authentication error.");
			}
			else if (!isAlNumStr(*(it + 1)))
			{
				sendToClient("Nicknames may only include alphanumerical characters.");
			}
			else if (findClient(*(it + 1)) != myClients.end())
			{
				sendToClient("Nickname is already registered.");
			}
			else
			{
				client_it->nickname = (*(it + 1));
				sendToClient("Your nick has been set.");
				std::cout << client_it->nickname << std::endl;
			}
		}
		else
		{
			sendToClient("Wrong formatting. use NICK <nickname>");
		}
	}
	
}

int Server::createSocket()
{
    serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    return 0;
}

int Server::setSocketOpt()
{
    if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR , &options, sizeof(options)) == 0)
    {
        std::cout << "set socket option successfull.\n";
        return IRC_OK;
    }
    else
    {
        std::cerr << "Error.\n";
        return IRC_FAIL;
    }
}

int Server::setNonBlock()
{
    if(fcntl(this->serverSocketFd, F_SETFL, O_NONBLOCK) < 0)
    {
        return -1;
    }
    return 0;
}

int Server::bindSocket()
{
    serverSocketAddress.sin_addr.s_addr = INADDR_ANY;
    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_port = htons(port);
    int ret = bind(serverSocketFd, reinterpret_cast<const struct sockaddr *>(&serverSocketAddress), sizeof(serverSocketAddress));
    if (ret == 0)
    {
        std::cout << "Bind is successfull.\n";
        return IRC_OK;
    }
    else
    {
        std::cerr << "Bind is failed.\n";
        return IRC_FAIL;
    }
}

int Server::listenSocket()
{
    int ret = listen(serverSocketFd, MAX_CLIENTS);
    if (ret == 0)
    {
        std::cout << "Listening is successfull.\n";
        return IRC_OK;
    }
    else
    {
        std::cerr << "Listening is failed.\n";
        return IRC_FAIL;
    }
}

int Server::acceptSocket()
{
    if((newClientFd = accept(serverSocketFd, (struct sockaddr *)&newClientSocketaddress, &newClientSocketAddressLen)) < 0)
    {
        std::cerr << "Accept Error.\n";
        return IRC_FAIL;
    }
    return IRC_OK;
}

std::vector<client_t>::iterator Server::findClient(std::string str)
{
	for(std::vector<client_t>::iterator it = myClients.begin(); it != myClients.end(); it++)
	{
		if ((*it).nickname == str)
			return it;
	}
	return myClients.end();
}

std::vector<client_t>::iterator Server::findClient(int fd)
{
	for(std::vector<client_t>::iterator it = myClients.begin(); it != myClients.end(); it++)
	{
		if ((*it).socketFd == fd)
			return it;
	}
	return myClients.end();
}

void Server::sendToClient(std::string str)
{
	send(newClientFd, (str + "\n").c_str(), str.size() + 1, 0);
}

std::string err_to_name(int err)
{
    switch (err)
    {
    case IRC_INVALID_ARG:
        return std::string("IRC_INVALID_ARG");
    case IRC_INVALID_PASSWORD:
        return std::string("IRC_INVALID_PASSWORD");
    default:
        return std::string("IRC_FAIL");
    }
    return std::string("FAIL");
}

bool Server::isAlNumStr(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!std::isalnum(str[i]))
			return false;
	}
	return true;
}
