#include "Server.h"

void	Server::pass(std::vector<std::string> &tokens)
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

void	Server::nick(std::vector<std::string> &tokens)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<client_t>::iterator client_it = findClient(newClientFd);

	std::cout << "call to NICK" << std::endl;
	
	if (tokens.size() == 2)
	{
		if (!isAlNumStr(*(tokens_it + 1)))
		{
			sendReply(ERR_NICK(*(tokens_it + 1)));
		}
		else if (findClient(*(tokens_it + 1)) != myClients.end())
		{
			sendReply(ERR_ALREADYREGISTERED(*(tokens_it + 1)));
		}
		else
		{
			for(std::vector<channel_t>::iterator it = myChannels.begin(); it != myChannels.end(); it++)
			{
				std::vector<client_t>::iterator ch_client_it = findClientInChannel(it, client_it->nickname);
				if (ch_client_it != it->operator_array.end())
				{
					ch_client_it->nickname = (*(tokens_it + 1));
				}
			}
			sendToClient(RPL_NICK(client_it->nickname, client_it->username, (*(tokens_it + 1))));
			client_it->nickname = (*(tokens_it + 1));
			std::cout << client_it->nickname << std::endl;
		}
	}
	else
	{
		sendReply(": use NICK <nickname>");
	}
}

void	Server::user(std::vector<std::string> &tokens)
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

void	Server::privmsg(std::vector<std::string> &tokens)
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
				sendToClientsInChannel(channel_it, RPL_PRIV(client_it->nickname, client_it->username, channel_it->name, (*(tokens_it + 2))));
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

void	Server::join(std::vector<std::string> &tokens)
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
				channel_it->operator_array.push_back(*client_it);
				sendToClientsInChannel(channel_it, RPL_JOIN(client_it->nickname, client_it->username, *(tokens_it + 1)));
				sendToClient(RPL_JOIN(client_it->nickname, client_it->username, *(tokens_it + 1)));
				if (channel_it->topic.length() == 0)
					sendReply(RPL_NOTOPIC(client_it->nickname, *(tokens_it + 1)));
				else
					sendReply(RPL_TOPIC(client_it->nickname, *(tokens_it + 1), channel_it->topic));
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
					sendReply(RPL_EONL(client_it->nickname, *(tokens_it + 1)));
				}
				else
					sendToClient(RPL_MODE(client_it->nickname, client_it->username, *(tokens_it + 1)));
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

void	Server::kick(std::vector<std::string> &tokens)
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

void	Server::quit(std::vector<std::string> &tokens)
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

void	Server::cap(std::vector<std::string> &tokens)
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

void Server::whoCommand(std::vector<std::string> &tokens)
{
    std::vector<std::string>::iterator token_it = tokens.begin();
    std::vector<client_t>::iterator client_it = findClient(newClientFd);



    if(tokens.size() == 1)
    {
        for(std::vector<client_t>::iterator it = myClients.begin(); it != myClients.end(); it++)
        {
            if (client_it->nickname != it->nickname)
                sendToClient("Nick: " + it->nickname);
        }
        sendToClient(": 315 " + tokens[1] + ": End of WHO list");
    }
    else if (tokens.size() == 2 && tokens[1][0] == '#')
    {
        std::vector<channel_t>::iterator channel_it = findChannel(*(token_it + 1));
        if (channel_it == myChannels.end())
                sendToClient (NO_SUCH_CHANNEL(client_it->nickname, "WHO"));
        else if (findClientInChannel(channel_it, client_it->nickname) == channel_it->operator_array.end())  // ??? Gerekli mi
            sendToClient (NOTONCHANNEL(client_it->nickname, tokens[1]));
        else
        {
            for(std::vector<client_t>::iterator it = channel_it->operator_array.begin(); it != channel_it->operator_array.end(); it++)
            {
                if (client_it->nickname != it->nickname)
                    sendToClient("Nick: " + it->nickname);
            }
            sendToClient(": 315 " + tokens[1] + ": End of WHO list");
        }
    }
    else
    {
        sendToClient("Invalid Usage: WHO [<channel>] or WHO");
    }
}

void Server::topicCommand(std::vector<std::string> &tokens)
{
    std::vector<std::string>::iterator token_it = tokens.begin();
    std::vector<client_t>::iterator client_it = findClient(newClientFd);
    std::vector<channel_t>::iterator channel_it = findChannel(*(token_it + 1));

    if (tokens[2][0] != ':' || tokens[1][0] != '#')
        sendToClient ("Invalid Usage. TOPIC <channel> :[<topic>]");
    else if (client_it->is_registered == false)
        sendReply(": use PASS-NICK-USER before sending any other commands");
    else
    {       
            prependColumn(tokens);
            if (channel_it == myChannels.end())
                sendToClient (NO_SUCH_CHANNEL(client_it->nickname, "TOPIC"));
            else if (findClientInChannel(channel_it, client_it->nickname) == channel_it->operator_array.end())
                sendToClient (NOTONCHANNEL(client_it->nickname, tokens[1]));
            else
            {
                channel_it->topic = tokens[2];
                sendToClientsInChannel(channel_it , TOPICCHANGED(client_it->nickname, client_it->username, tokens[1], tokens[2]));
				sendToClient(TOPICCHANGED(client_it->nickname, client_it->username, tokens[1], tokens[2]));
            }
    }
}

void Server::partCommand(std::vector<std::string> &tokens)
{
    std::vector<std::string>::iterator token_it = tokens.begin();
    std::vector<client_t>::iterator client_it = findClient(newClientFd);
    std::vector<channel_t>::iterator channel_it = findChannel(*(token_it + 1));


    if (tokens[1][0] != '#' || tokens[1].size() < 2 || !((tokens.size() >= 3 && tokens[2][0] == ':') || tokens.size() == 2))
        sendToClient (": use PART <channel>");
    else if (client_it->is_registered == false)
        sendReply(": use PASS-NICK-USER before sending any other commands");
    else
    {       
            if (tokens.size() >= 3)
                prependColumn(tokens);
            if (channel_it == myChannels.end())
                sendReply (NO_SUCH_CHANNEL(client_it->nickname, "PART"));
            else if (findClientInChannel(channel_it, client_it->nickname) == channel_it->operator_array.end())
                sendReply (NOTONCHANNEL(client_it->nickname, tokens[1]));
            else
            {
                channel_it->operator_array.erase(findClientInChannel(channel_it, client_it->nickname));
                if (tokens.size() >= 3)
				{
                    sendToClientsInChannel(channel_it, PARTWITHREASON(client_it->nickname, client_it->username, tokens[1], tokens[2]));
					sendToClient(PARTWITHREASON(client_it->nickname, client_it->username, tokens[1], tokens[2]));
				}
                else
				{
                    sendToClientsInChannel(channel_it , PART(client_it->nickname, client_it->username, tokens[1]));
					sendToClient(PART(client_it->nickname, client_it->username, tokens[1]));
				}
				
            }
    }
}

void Server::noticeCommand(std::vector<std::string> &tokens)
{
    std::vector<std::string>::iterator token_it = tokens.begin();
    std::vector<client_t>::iterator client_it = findClient(newClientFd);
    std::string ret;

    prependColumn(tokens);
    if (tokens.size() == 3 && (*(token_it + 2))[0] == ':' && (*(token_it + 2)).length() >= 2)
    {
        *(token_it + 2) = (*(token_it + 2)).substr(1, (*(token_it + 2)).size() - 1);
        if (client_it->is_registered == false)
        {
            sendReply(": use PASS-NICK-USER before sending any other commands");
        }
        else if (findClient(*(token_it + 1)) == myClients.end() || findClient(*(token_it + 1))->is_registered == false)
        {
            sendReply(NO_NICKNAME(client_it->nickname, *(token_it + 1)));
        }
        else
        {
            sendToClient(findClient(*(token_it + 1))->socketFd, ":" + client_it->nickname + "!" + client_it->username + "@localhost NOTICE " + token_it[1] + " :" + (*(token_it + 2)));
            
        }
    }
    else
    {
        ret = ("Incorrect format.");
        ret.append(" NOTICE <msgtarget> :<message>");
    }
}