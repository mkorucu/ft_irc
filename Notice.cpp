#include "Server.h"

void Server::noticeCommand(std::vector<std::string> tokens)
{
    std::vector<std::string>::iterator token_it = tokens.begin();
    std::vector<client_t>::iterator client_it = findClient(newClientFd);
    std::string ret;

    prependColumn(tokens);
    if (tokens.size() == 3 && (*(token_it + 2))[0] == ':' && (*(token_it + 2)).length() >= 2)
    {
        *(token_it + 2) = (*(token_it + 2)).substr(1, sizeof(*(token_it + 2)) - 1);
        if (client_it->is_registered == false)
        {
            sendToClient("Authentication error.");
        }
        else if ((*(token_it + 1))[0] == '#')
        {
            std::vector<channel_t>::iterator channel_it = findChannel(*(token_it + 1));
            if (channel_it == myChannels.end())
            {
                sendToClient("Channel not found.");
            }
            else if (findClientInChannel(channel_it, client_it->nickname) == channel_it->operator_array.end())
            {
                sendToClient("You are not in channel");
            }
            else
            {
                std::string sender = client_it->nickname;
                for(client_it = channel_it->operator_array.begin(); client_it != channel_it->operator_array.end(); client_it++)
                {
                    sendToClient(client_it->socketFd, sender + channel_it->name + " :" + (*(token_it + 2)));
                }
            }
        }
        else if (findClient(*(token_it + 1)) == myClients.end() || findClient(*(token_it + 1))->is_registered == false)
        {
            sendToClient("User not found.");
        }
        else
        {
            sendToClient(findClient(*(token_it + 1))->socketFd, client_it->nickname + ": " + (*(token_it + 2)));
        }
    }
    else
    {
        ret = ("Incorrect format.");
        ret.append(" NOTICE <msgtarget> :<message>");
    }
}