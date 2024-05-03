#include "Server.h"

void	Server::pass(std::vector<std::string> tokens)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<client_t>::iterator client_it = findClient(newClientFd);

	std::cout << "call to PASS" << std::endl; 
	if (tokens.size() == 2)
	{
		if (client_it->is_auth == true)
			;
		else if (*(tokens_it + 1) != ":" + this->password)
			sendReply(PASS_ERR());
		else
			client_it->is_auth = true;
	}
	else
		sendReply(": use PASS :password");
}

void	Server::nick(std::vector<std::string> tokens)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<client_t>::iterator client_it = findClient(newClientFd);

	std::cout << "call to NICK" << std::endl;
	
	if (tokens.size() == 2)
	{
		if (!isAlNumStr(*(tokens_it + 1)))
		{
			sendReply(": Nicknames may only include alphanumerical characters.");
		}
		else if (findClient(*(tokens_it + 1)) != myClients.end())
		{
			sendReply(ERR_ALREADYREGISTERED(*(tokens_it + 1)));
		}
		else
		{
			client_it->nickname = (*(tokens_it + 1));
			for(std::vector<channel_t>::iterator it = myChannels.begin(); it != myChannels.end(); it++)
			{
				client_it = findClientInChannel(it, client_it->nickname);
				if (client_it != it->operator_array.end())
				{
					std::cout << "client nickname updated on channel " << it->name << std::endl;
					client_it->nickname = (*(tokens_it + 1));
				}
			}
			std::cout << client_it->nickname << std::endl;
		}
	}
	else
	{
		sendReply(": use NICK <nickname>");
	}
}

void	Server::user(std::vector<std::string> tokens)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<client_t>::iterator client_it = findClient(newClientFd);

	std::cout << "call to NICK" << std::endl;
	prependColumn(tokens);
	if (tokens.size() == 5 && (*(tokens_it + 4))[0] == ':' && (*(tokens_it + 4)).length() >= 2)
	{
		*(tokens_it + 4) = (*(tokens_it + 4)).substr(1, sizeof(*(tokens_it + 4)) - 1);
		if (client_it->username.empty() == false || client_it->real_name.empty() == false)
		{
			sendReply(": you are already registered!");
		}
		else
		{
			client_it->username = (*(tokens_it + 1));
			client_it->real_name = (*(tokens_it + 4));
			client_it->is_registered = true;

			sendReply(RPL_WELCOME(client_it->nickname, client_it->username, this->hostname));
			sendReply(RPL_YOURHOST(client_it->nickname, this->hostname));
			sendReply(RPL_CREATED(client_it->nickname, this->c_date));
		}
	}
	else
	{
		sendReply(": use USER <username> <mode> <unused> :realname");
	}
}

void	Server::privmsg(std::vector<std::string> tokens)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<client_t>::iterator client_it = findClient(newClientFd);

	prependColumn(tokens);
	if (tokens.size() == 3 && (*(tokens_it + 2))[0] == ':' && (*(tokens_it + 2)).length() >= 2)
	{
		*(tokens_it + 2) = (*(tokens_it + 2)).substr(1, (*(tokens_it + 2)).length() - 1);
		if (client_it->is_registered == false || client_it->is_auth == false || client_it->nickname.length() == 0)
			sendToClient(": use PASS-NICK-USER before sending any other commands");
		else if ((*(tokens_it + 1))[0] == '#')
		{
			std::vector<channel_t>::iterator channel_it = findChannel(*(tokens_it + 1));
			if (channel_it == myChannels.end())
			{
				sendReply(": Channel not found.");
			}
			else if (findClientInChannel(channel_it, client_it->nickname) == channel_it->operator_array.end())
			{
				sendReply(": You are not in channel");
			}
			else
			{
				std::string sender = client_it->nickname;
				for(client_it = channel_it->operator_array.begin(); client_it != channel_it->operator_array.end(); client_it++)
				{
					sendToClient(client_it->socketFd, sender + channel_it->name + " :" + (*(tokens_it + 2)));
				}
			}
		}
		else if (findClient(*(tokens_it + 1)) == myClients.end() || findClient(*(tokens_it + 1))->is_registered == false)
		{
			sendToClient(": User not found.");
		}
		else
		{
			sendToClient(findClient(*(tokens_it + 1))->socketFd, client_it->nickname + ": " + (*(tokens_it + 2)));
		}
	}
	else
	{
		sendToClient(": use PRIVMSG <msgtarget> <text to be sent>");
	}

}

void	Server::join(std::vector<std::string> tokens)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<client_t>::iterator client_it = findClient(newClientFd);

	if (tokens.size() == 2 && (*(tokens_it + 1))[0] == '#' && (*(tokens_it + 1)).length() >= 2)
	{
		std::vector<channel_t>::iterator channel_it = findChannel(*(tokens_it + 1));
		
		if (client_it->is_registered == false || client_it->is_auth == false || client_it->nickname.length() == 0)
			sendReply(": use PASS-NICK-USER before sending any other commands");
		else if (channel_it != myChannels.end())
		{
			if (findClientInChannel(channel_it, client_it->nickname) != channel_it->operator_array.end())
				;
			else
			{
				if (channel_it->topic.length() == 0)
				{
					sendToClient(RPL_JOIN(client_it->nickname, client_it->username, *(tokens_it + 1))); //send to everyone
					sendReply(RPL_NOTOPIC(client_it->nickname, *(tokens_it + 1)));
					if (channel_it->operator_array.size() > 0)
					{
						std::vector<client_t>::iterator it = channel_it->operator_array.begin();
						std::string users = it->nickname;
						it++;
						while (it != channel_it->operator_array.end())
						{
							users += (" " + it->nickname);
							it++;
						}
						sendReply(RPL_USRS(client_it->nickname, *(tokens_it + 1), users));
					}
					sendReply(RPL_EONL(client_it->nickname, *(tokens_it + 1)));
				}
				else
				{
					sendToClient(RPL_JOIN(client_it->nickname, client_it->username, *(tokens_it + 1))); //send to everyone
					sendReply(RPL_TOPIC(client_it->nickname, *(tokens_it + 1), channel_it->topic));
				}
				channel_it->operator_array.push_back(*client_it);
			}
		}
		else 
		{
			channel_t *chan = new channel_t;
			chan->name = *(tokens_it + 1);
			chan->operator_array.push_back(*client_it);
			myChannels.push_back(*chan);
			sendToClient(RPL_JOIN(client_it->nickname, client_it->username, *(tokens_it + 1)));
			sendToClient(RPL_MODE(client_it->nickname, client_it->username, *(tokens_it + 1)));
			sendReply(RPL_NOTOPIC(client_it->nickname, *(tokens_it + 1)));
			delete chan;
		}
	}
	else
	{
		sendReply(": use JOIN #channel");
	}
}

void	Server::kick(std::vector<std::string> tokens)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<client_t>::iterator client_it = findClient(newClientFd);

	if (tokens.size() == 3 && (*(tokens_it + 1))[0] == '#' && (*(tokens_it + 1)).length() >= 2)
	{
		std::vector<channel_t>::iterator channel_it = findChannel(*(tokens_it + 1));
		
		if (client_it->is_registered == false || client_it->is_auth == false || client_it->nickname.length() == 0)
			sendReply(": use PASS-NICK-USER before sending any other commands");
		else if (channel_it != myChannels.end())
		{
			std::cout << "channel " << channel_it->name << " | first index nick: " << channel_it->operator_array[0].nickname << std::endl;
			if (findClientInChannel(channel_it, client_it->nickname) == channel_it->operator_array.end())
				sendReply(": You are not in Channel.");
			else if (findClientInChannel(channel_it, client_it->nickname) != channel_it->operator_array.begin())
				sendReply(": You are not an operator!");
			else if (findClientInChannel(channel_it, (*(tokens_it + 2))) == channel_it->operator_array.end())
				sendReply(": User not in channel.");
			else
			{
				std::__1::vector<client_t>::iterator client_to_kick = findClientInChannel(channel_it, (*(tokens_it + 2)));

				channel_it->operator_array.erase(client_to_kick);
				// kicked sendToClient(client_to_kick->socketFd, "You are kicked by " + client_it->nickname);
			}
		}
		else
		{
			sendReply(": Channel not found!");
		}
	}
	else
	{
		sendReply(": use KICK #channel <nickname>");
	}
}

void	Server::quit(std::vector<std::string> tokens)
{
	if (tokens.size() == 1)
	{
		close(newClientFd);
		deleteClientOnEverywhere(newClientFd);
		FD_CLR(newClientFd, &current_sockets);
		std::cout << "Client " << newClientFd << " left.\n";
	}
	else
	{
		sendReply(": use QUIT");
	}
}

void	Server::cap(std::vector<std::string> tokens)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();

	if (tokens.size() >= 2 && (*(tokens_it + 1)) == "LS")
	{
		sendToClient("CAP * LS :multi-prefix sasl");
	}
	else if (tokens.size() >= 2 && (*(tokens_it + 1)) == "REQ")
	{
		sendToClient("CAP * ACK multi-prefix");
	}
	else if (tokens.size() >= 2 && (*(tokens_it + 1)) == "END")
		;
}
