#ifndef __ERROR_MESSAGES_H
#define __ERROR_MESSAGES_H


#define RPL_WELCOME(nick, user, host) (" 001 " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host)
#define RPL_YOURHOST(nick, host) (" 002 " + nick + " :Your host is " + host + ", running version v1.0.0")
#define RPL_CREATED(nick, date) (" 003 " + nick + " :This server was created " + date)

#define RPL_NOTOPIC(nick, channel) (" 331 " + nick + " " + channel + " :No topic is set")
#define RPL_TOPIC(nick, channel, topic) (" 332" + nick + " " + channel + " :" + topic)
#define RPL_JOIN(nick, user, channel) (":" + nick + "!" + user + "@*" + " JOIN " + channel)
#define RPL_PRIV(nick, user, channel, msg) (":" + nick + "!" + user + "@*" + " PRIVMSG " + channel + " :" + msg)
#define RPL_MODE(nick, user, channel) (":" + nick + "!" + user + "@*" + " JOIN " + channel + " +o " + nick)
#define RPL_USRS(nick, channel, users) (" 353 " + nick + " = " + channel + " :@" + users)
#define RPL_EONL(nick, channel) (" 366 " + nick + " " + channel + " :End of /NAMES list")



//soner
#define ERR_USERNOTINCHANNEL(nickname, username) (" 482 " + client + " " + nickname + " :You're not channel operator")
#define ERR_CHANOPRIVSNEEDED(notyournick, username) (" 441 " + client + " " + nickname + " :They aren't on that channel")
#define NOTONCHANNEL(client, channel) (" 442 " + client + " " + channel + " :You're not on that channel")
#define NO_SUCH_CHANNEL(nickname, channel) (" 403 " + nickname + " " + channel + " :No such channel")
#define USERONCHANNEL(client, channel) (" 443 " + client + " :cannot join because " + channel)
#define NO_NICKNAME(client, nickname) (" 401 " + client + " " + nickname + " :No such nick")
#define NICKNAME_IN_USE(nickname) (" 433 " + nickname + " :Nickname is already in use. Please provide your password with 'USERPASS: <password> if you have this nickname already!'")
#define ERR_ALREADYREGISTERED(nickname) (" 462 " + nickname + " :Already registered."  )

#define TOPICCHANGED(nickname, username, channelname, topic) (nickname + "!" + username +"@localhost TOPIC "+ channelname , " :" + topic)
#define PART(nickname, username, command,channelname) (":"+ nickname + "!" + username +"@localhost " + command + " " + channelname)
#define PARTWITHREASON(nickname, username, command,channelname, reason) (":"+ nickname + "!" + username +"@localhost " + command + " " + channelname + " " + reason)


#define PASS_ERR() (" 464 :Password incorrect! Please check your password")
#define ERR_PASSWDMISMATCH(nick) (" 464 " + nick + "  : password supplied does not match the password expected by the server!")


#endif