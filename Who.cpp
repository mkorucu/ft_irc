#include "Server.h"

void Server::whoCommand(std::vector<std::string> tokens)
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
                sendToClient (NO_SUCH_CHANNEL(client_it->nickname, tokens[1]));
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