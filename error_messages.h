#ifndef __ERROR_MESSAGES_H
#define __ERROR_MESSAGES_H


#define ERR_USERNOTINCHANNEL(nickname, username) (": 482 " + client + " " + nickname + " :You're not channel operator\r\n")
#define ERR_CHANOPRIVSNEEDED(notyournick, username) (": 441 " + client + " " + nickname + " :They aren't on that channel\r\n")
#define NOTONCHANNEL(client, channel) (": 442 " + client + " " + channel + " :You're not on that channel\r\n")
#define NO_SUCH_CHANNEL(nickname, channel) (": 403 " + nickname + " " + channel + " :No such channel\r\n")
#define USERONCHANNEL(client, channel) (": 443 " + client + " :cannot join because " + channel + "\r\n")
#define NO_NICKNAME(client, nickname) (": 401 " + client + " " + nickname + " :No such nick\r\n")
#define NICKNAME_IN_USE(nickname) (": 433 " + nickname + " :Nickname is already in use. Please provide your password with 'USERPASS: <password> if you have this nickname already!'\r\n")
#define ERR_ALREADYREGISTERED(nickname) (": 462 " + nickname + " :Already registered.\r\n"  )
#define LOGIN(nickname, username) (": 001 " + nickname + " :Welcome to the IRC Network " + nickname + "!" + username + "\r\n")

#define TOPICCHANGED(nickname, username, channelname, topic) (":"+ nickname + "!" + username +"@localhost TOPIC "+ channelname , " :"+topic)
#define PART(nickname, username, command,channelname) (":"+ nickname + "!" + username +"@localhost " + command + " " + channelname)
#define PARTWITHREASON(nickname, username, command,channelname, reason) (":"+ nickname + "!" + username +"@localhost " + command + " " + channelname + " " + reason)

#define PASS_ERR(nickname) (": 464 " + nickname + " :Password incorrect! Please check your password\r\n")
#define ERR_PASSWDMISMATCH(nick) (": 464 " + nick + "  : password supplied does not match the password expected by the server!\r\n")
#define BADCHANNEL(channel) (": 475 " + channel + " :Incorrect password for channel!\r\n")


#endif