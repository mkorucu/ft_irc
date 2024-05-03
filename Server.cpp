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

	char hostname_c[64];
	gethostname(hostname_c, 64);
	this->hostname = hostname_c;

	this->c_date = getTime();

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
                        client_t *client = new client_t;
						client->socketFd = newClientFd;
                        myClients.push_back(*client);
						delete client;
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
					std::vector<client_t>::iterator client_it = findClient(newClientFd);
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
						deleteClientOnEverywhere(newClientFd);
                    }
					else if (buff[read_val - 1] != '\n')
						client_it->input_buff.append(buff);
                    else if (read_val == 1 && *buff == '\n' && client_it->input_buff.size() == 0) // '\n'
                        continue;
                    else
                    {
						buff[read_val - 1] = '\0';
						client_it->input_buff.append(buff);
						std::cout << "----------------------------\n";
						std::cout << "\033[32m" << "Client " << newClientFd << ": " << "\033[0m" << client_it->input_buff << std::endl;
						parseClient();
						client_it->input_buff.clear();
						client_it->input_buff.shrink_to_fit();
						std::cout << "----------------------------\n";
                    }
					memset(buff, 0, sizeof(buff));
                }
            }
        }
    }
}

void Server::parseClient()
{
	std::vector<client_t>::iterator client_it = findClient(newClientFd);
    std::istringstream iss(client_it->input_buff);
	std::string line;
	std::vector<std::string> lines;


	while (std::getline(iss, line, '\n'))
	{
		if (line.length() == 0 || line == "\n")
			continue ;
		else if (line[line.length() - 1] == '\r')
			lines.push_back(line.substr(0, line.length() - 1));
		else
    		lines.push_back(line);
    }
	std::vector<std::string>::iterator lines_it = lines.begin();

	while (lines_it != lines.end())
	{
		std::string token;
		std::vector<std::string> tokens;
		std::istringstream isslines(*lines_it);

		while (std::getline(isslines, token, ' '))
		{
			if (token.length() == 0 || token == "\n")
				continue ;
    		tokens.push_back(token);
    	}
	
		std::vector<std::string>::iterator tokens_it = tokens.begin();
		if (*tokens_it == "PASS")
			pass(tokens);
		else if (*tokens_it == "JOIN")
			join(tokens);
		else if (*tokens_it == "NICK")
			nick(tokens);
		else if (*tokens_it == "USER")
			user(tokens);
		else if (*tokens_it == "PRIVMSG")
			privmsg(tokens);
		else if (*tokens_it == "KICK")
			kick(tokens);
		else if (*tokens_it == "QUIT")
			quit(tokens);
		else if (*tokens_it == "CAP")
			cap(tokens);
		else if (*tokens_it == "NOTICE")
			noticeCommand(tokens);
		else if (*tokens_it == "PART")
			partCommand(tokens);
		else if (*tokens_it == "TOPIC")
			topicCommand(tokens);
		else if (*tokens_it == "WHO")
			whoCommand(tokens);
		else
			;
		lines_it++;
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

std::vector<client_t>::iterator Server::findClient(std::string &str)
{
	for(std::vector<client_t>::iterator it = myClients.begin(); it != myClients.end(); it++)
	{
		if ((*it).nickname == str)
			return it;
	}
	return myClients.end();
}

std::vector<client_t>::iterator Server::findClientInChannel(std::vector<channel_t>::iterator channel, std::string &str)
{
	std::vector<client_t>::iterator it = channel->operator_array.begin();
	std::vector<client_t>::iterator end = channel->operator_array.end();
	while(it != end)
	{
		if (it->nickname == str)
			return it;
		it++;
	}
	return it;
}

std::vector<channel_t>::iterator Server::findChannel(std::string &name)
{
	for(std::vector<channel_t>::iterator it = myChannels.begin(); it != myChannels.end(); it++)
	{
		if ((*it).name == name)
			return it;
	}
	return myChannels.end();
}

void Server::deleteClientOnEverywhere(int client_fd)
{
	std::vector<client_t>::iterator client_it = findClient(client_fd);
	if (client_it != myClients.end())
	{
		for(std::vector<channel_t>::iterator it = myChannels.begin(); it !=  myChannels.end(); it++)
		{
			std::vector<client_t>::iterator client = findClientInChannel(it, client_it->nickname);
			if (client != it->operator_array.end())
				it->operator_array.erase(client);
		}
		myClients.erase(client_it);
	}
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
	send(newClientFd, (str + "\r\n").c_str(), str.size() + 2, 0);
}

void Server::sendToClient(int fd, std::string str)
{
	send(fd, (str + "\r\n").c_str(), str.size() + 2, 0);
}

void Server::sendToClientsInChannel(std::vector<channel_t>::iterator channel_it, std::string str)
{
	for(std::vector<client_t>::iterator clients_it = channel_it->operator_array.begin(); clients_it != channel_it->operator_array.end(); clients_it++)
	{
		if (clients_it->socketFd != newClientFd)
			send(clients_it->socketFd, (str + "\n").c_str(), str.size() + 1, 0);
	}
}

void Server::sendReply(std::string str)
{
	std::string reply = ":" + this->hostname + str + "\r\n";
	send(newClientFd, reply.c_str(), reply.size(), 0);
	std::cout << ">>>" << str << std::endl;
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

std::string Server::getTime()
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];
	std::time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "%d-%m-%Y %I:%M:%S", timeinfo);
	return std::string(buffer);
}