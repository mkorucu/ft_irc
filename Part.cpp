#include "Server.h"

void Server::partCommand(std::vector<std::string> tokens)
{
    std::vector<std::string>::iterator token_it = tokens.begin();
    std::vector<client_t>::iterator client_it = findClient(newClientFd);
    std::vector<channel_t>::iterator channel_it = findChannel(*(token_it + 1));


    if (tokens[1][0] != '#' || tokens[1].size() < 2 || !((tokens.size() >= 3 && tokens[2][0] == ':') || tokens.size() == 2))
        sendToClient ("Invalid Usage. PART <channel>");
    else if (client_it->is_registered == false)
        sendToClient ("AuthError.");
    else
    {       
            if (tokens.size() >= 3)
                prependColumn(tokens);
            if (channel_it == myChannels.end())
                sendToClient (NO_SUCH_CHANNEL(client_it->nickname, tokens[1]));
            else if (findClientInChannel(channel_it, client_it->nickname) == channel_it->operator_array.end())
                sendToClient (NOTONCHANNEL(client_it->nickname, tokens[1]));
            else
            {
                if (tokens.size() >= 3)
                    sendToClientsInChannel(channel_it, PARTWITHREASON(client_it->nickname, client_it->username, tokens[0], tokens[1], tokens[2]));
                else    
                    sendToClientsInChannel(channel_it , PART(client_it->nickname, client_it->username, tokens[0], tokens[1]));
                channel_it->operator_array.erase(findClientInChannel(channel_it, client_it->nickname));
            }
    }
}