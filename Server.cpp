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
						parseClient();
                    }
                }
            }
        }
        // for(size_t i = 0; i < myChannels.size(); i++)
        // {
        //     for(size_t j = 0; j < myChannels.at(i).operator_array.size(); j++)
        //     {
        //         std::cout << myChannels.at(i).name << " operator is: " << myChannels.at(i).operator_array.at(j).nickname << std::endl;
        //     }
        // }
    }
}

void Server::parseClient()
{
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
		std::cout << "Pass is invoked" << std::endl; 
		if (tokens.size() == 2)
		{
			if (client_it->is_auth == true)
			{
				sendToClient("Already authorized.");
			}
			else if (*(it + 1) != this->password)
			{
				sendToClient(PASS_ERR(client_it->nickname));
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
		std::cout << "NICK is invoked" << std::endl;
		
		if (tokens.size() == 2)
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
				sendToClient(ERR_ALREADYREGISTERED(*(it + 1)));
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
	else if ((*it) == "USER")
	{
		std::cout << "USER is invoked" << std::endl;

		prependColumn(tokens);
		if (tokens.size() == 5 && (*(it + 4))[0] == ':' && (*(it + 4)).length() >= 2)
		{
			*(it + 4) = (*(it + 4)).substr(1, sizeof(*(it + 4)) - 1);
			if (client_it->is_auth == false || client_it->nickname.empty())
			{
				sendToClient("Authentication error.");
			}
			else if (isAlNumStr(*(it + 1)) == false || isAlNumSpStr(*(it + 4)) == false)
			{
				sendToClient("Usernames and realnames may only include alphanumerical characters.");
			}
			else if (client_it->username.empty() == false || client_it->real_name.empty() == false)
			{
				sendToClient("You are already registered!");
			}
			else
			{
				client_it->username = (*(it + 1));
				client_it->real_name = (*(it + 4));
				client_it->is_registered = true;
				sendToClient("Welcome to Internet Relay Chat!");
			}
		}
		else
		{
			sendToClient("Incorrect format.");
			sendToClient("use USER <username> <mode> <unused> <realname>");
		}
	}
	else if ((*it) == "QUIT")
	{
		std::cout << "QUIT invoked" << std::endl;
		if (tokens.size() == 1)
		{
			close(newClientFd);
			myClients.erase(findClient(newClientFd));
			FD_CLR(newClientFd, &current_sockets);
			std::cout << "Client " << newClientFd << " left.\n";
		}
		else
		{
			sendToClient("Wrong formatting. use QUIT");
		}
	}
	else if ((*it) == "PRIVMSG")
	{
		prependColumn(tokens);
		if (tokens.size() == 3 && (*(it + 2))[0] == ':' && (*(it + 2)).length() >= 2)
		{
			*(it + 2) = (*(it + 2)).substr(1, sizeof(*(it + 2)) - 1);
			if (client_it->is_registered == false)
			{
				sendToClient("Authentication error.");
			}
			else if ((*(it + 1))[0] == '#')
			{
				std::vector<channel_t>::iterator channel_it = findChannel(*(it + 1));
				if (channel_it == myChannels.end())
				{
					sendToClient("Channel not found.");
				}
				else if (findClientInChannel(channel_it->operator_array.begin(), channel_it->operator_array.end(), client_it->nickname) == channel_it->operator_array.end())
				{
					sendToClient("You are not in channel");
				}
				else
				{
					std::string sender = client_it->nickname;
					for(client_it = channel_it->operator_array.begin(); client_it != channel_it->operator_array.end(); client_it++)
					{
						sendToClient(client_it->socketFd, sender + channel_it->name + " :" + (*(it + 2)));
					}
				}
			}
			else if (findClient(*(it + 1)) == myClients.end() || findClient(*(it + 1))->is_registered == false)
			{
				sendToClient("User not found.");
			}
			else
			{
				sendToClient(findClient(*(it + 1))->socketFd, client_it->nickname + ": " + (*(it + 2)));
			}
		}
		else
		{
			sendToClient("Incorrect format.");
			sendToClient("PRIVMSG <msgtarget> <text to be sent>");
		}
	}

	else if ((*it) == "JOIN")
	{
		if (tokens.size() == 2 && (*(it + 1))[0] == '#' && (*(it + 1)).length() >= 2)
		{
			std::vector<channel_t>::iterator channel_it = findChannel(*(it + 1));
			
			if (client_it->is_registered == false)
				sendToClient("AuthError.");
			else if (channel_it != myChannels.end())
			{
				std::cout << "client " << newClientFd << ", nick " << client_it->nickname << std::endl;
				if (findClientInChannel(channel_it->operator_array.begin(), channel_it->operator_array.end(), client_it->nickname) != channel_it->operator_array.end())
					sendToClient("Already in.");
				else
				{
					channel_it->operator_array.push_back(*client_it);

					sendToClient("Joined the channel..");
				}
			}
			else 
			{
				sendToClient("Joined the channel as Operator..");
				channel_t chan;
				chan.name = *(it + 1);
				chan.operator_array.push_back(*client_it);
				myChannels.push_back(chan);
				std::cout << "Channel " << chan.name << " created.\n";
			}
		}
		else
		{
			sendToClient("Invalid JOIN usage..");
		}
	}
	else if ((*it) == "KICK")
	{
		if (tokens.size() == 3 && (*(it + 1))[0] == '#' && (*(it + 1)).length() >= 2)
		{
			std::vector<channel_t>::iterator channel_it = findChannel(*(it + 1));
			
			if (client_it->is_registered == false)
				sendToClient("AuthError.");
			else if (channel_it != myChannels.end())
			{
				if (findClientInChannel(channel_it->operator_array.begin(), channel_it->operator_array.end(), client_it->nickname) != channel_it->operator_array.begin())
					sendToClient("You are not an operator!");
				else if (findClientInChannel(channel_it->operator_array.begin(), channel_it->operator_array.end(), (*(it + 2))) == channel_it->operator_array.end())
					sendToClient("User not in channel.");
				else
				{
					(*channel_it).operator_array.erase(findClientInChannel(channel_it->operator_array.begin(), channel_it->operator_array.end(), (*(it + 2))));
				}
			}
			else
			{
				sendToClient("Channel not found!");
			}
		}
		else
		{
			sendToClient("Invalid KICK usage..");
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

std::vector<client_t>::iterator Server::findClientInChannel(std::vector<client_t>::iterator it, std::vector<client_t>::iterator end, std::string str)
{
	while (it != end)
	{
		std::cout << "IN CLIENT ITERATOR CURRENT NAME: " << it->nickname << std::endl;

		if ((*it).nickname == str)
			return it;
		it++;
	}
	std::cout << "CANNOT FIND USER IN CHANNEL ARRAY\n";
	return it;
}

std::vector<channel_t>::iterator Server::findChannel(std::string name)
{
	for(std::vector<channel_t>::iterator it = myChannels.begin(); it != myChannels.end(); it++)
	{
		if ((*it).name == name)
			return it;
	}
	return myChannels.end();
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

void Server::sendToClient(int fd, std::string str)
{
	send(fd, (str + "\n").c_str(), str.size() + 1, 0);
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

bool Server::isAlNumSpStr(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!std::isalnum(str[i]) && str[i] != ' ')
			return false;
	}
	return true;
}

void Server::prependColumn(std::vector<std::string> &tokens)
{
	std::vector<std::string>::iterator it1 = tokens.begin();
	while (it1 != tokens.end())
	{
		if ((*it1)[0] == ':')
			break;
		it1++;
	}
	if (it1 == tokens.end())
		return ;
	while (it1 + 1 != tokens.end())
	{
		(*it1).append(" ");
		(*it1).append(*(it1 + 1));
		tokens.erase(it1 + 1);
	}
}