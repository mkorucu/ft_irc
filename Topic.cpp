#include "Server.h"

void Server::topicCommand(std::vector<std::string> tokens)
{
    std::vector<std::string>::iterator token_it = tokens.begin();
    std::vector<client_t>::iterator client_it = findClient(newClientFd);
    std::vector<channel_t>::iterator channel_it = findChannel(*(token_it + 1));

    if (tokens[2][0] != ':' || tokens[1][0] != '#')
        sendToClient ("Invalid Usage. TOPIC <channel> :[<topic>]");
    else if (client_it->is_registered == false)
        sendToClient ("AuthError.");
    else
    {       
            prependColumn(tokens);
            if (channel_it == myChannels.end())
                sendToClient (NO_SUCH_CHANNEL(client_it->nickname, tokens[1]));
            else if (findClientInChannel(channel_it, client_it->nickname) == channel_it->operator_array.end())
                sendToClient (NOTONCHANNEL(client_it->nickname, tokens[1]));
            else
            {
                channel_it->topic = tokens[2];
                sendToClientsInChannel(channel_it , TOPICCHANGED(client_it->nickname, client_it->username, tokens[1], tokens[2]));
            }
    }
}