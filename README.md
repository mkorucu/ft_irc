Ecole 42 project ft_irc
Internet Relay Chat server based on C++

To compile it:
  Make or Make run
To run it:
  ./ircserv <port> <server_password>

Commands:
  PASS :<password>
  NICK <nickname>
  USER <username> <flag1(can be set as 0)> <flag2(can be set as 0)> :<real_name>
  JOIN #<channel_name>
  PRIVMSG #<channel_name> :<message> or PRIVMSG <nickname> :<message>
  KICK #<channel_name> <nickname>
  PART #<channel_name> or PART #<channel_name> :<message>
  NOTICE <username> :<message>
  
