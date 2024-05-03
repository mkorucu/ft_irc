#ifndef __SERVER_H
#define __SERVER_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <iostream>
#include <fcntl.h>

#include "error_messages.h"

#define MAX_CLIENTS 10

typedef enum irc_error_t{
    IRC_FAIL = -1,
    IRC_OK = 0,
    IRC_INVALID_ARG,
    IRC_INVALID_PASSWORD,

}irc_error_t;

std::string err_to_name(int err);


#define IRC_ERROR_CHECK(x) if(x) std::cerr << err_to_name(x) << std::endl;


typedef struct client_t
{
    int socketFd;
    std::string nickname;
    std::string username;
    std::string real_name;
    std::string input_buff;
    bool        is_auth;
	bool		is_registered;
}client_t;

typedef struct channel_t
{
    std::string name;
    std::string topic;
    std::vector<client_t> operator_array;
}channel_t;

class Server
{   
    public:
        Server(std::string port, std::string password);
        int init();
        void start();

    private:
        void parseClient();
        int createSocket();
        int setSocketOpt();
        int setNonBlock();
        int bindSocket();
        int listenSocket();
        int acceptSocket();

		void	pass(std::vector<std::string> &tokens);
		void	nick(std::vector<std::string> &tokens);
		void	user(std::vector<std::string> &tokens);
		void	privmsg(std::vector<std::string> &tokens);
		void	join(std::vector<std::string> &tokens);
		void	kick(std::vector<std::string> &tokens);
		void	quit(std::vector<std::string> &tokens);
		void	cap(std::vector<std::string> &tokens);
	
        void noticeCommand(std::vector<std::string> &tokens);
        void topicCommand(std::vector<std::string> &tokens);
        void partCommand(std::vector<std::string> &tokens);
        void whoCommand(std::vector<std::string> &tokens);

        class InvalidArgument : public std::exception{
            public:
                const char *what() const throw(){
                    return "InvalidArgument.";
                }
        };

		bool isAlNumStr(std::string str);
		bool isAlNumSpStr(std::string str);

		std::string getTime();

		void sendToClient(std::string str);
		void sendToClient(int fd, std::string str);
		void sendToClientsInChannel(std::vector<channel_t>::iterator channel_it, std::string str);
		void sendReply(std::string str);
		std::vector<client_t>::iterator findClient(int fd);
		std::vector<client_t>::iterator findClient(std::string &str);
		std::vector<client_t>::iterator findClientInChannel(std::vector<channel_t>::iterator channel, std::string &str);
        std::vector<channel_t>::iterator findChannel(std::string &name);
        void deleteClientOnEverywhere(int client_fd);
		void prependColumn(std::vector<std::string> &tokens);
        int port;
        std::string password;
        sockaddr_in serverSocketAddress;
        int serverSocketFd;
        int options;

    	struct sockaddr_in	newClientSocketaddress;
	    socklen_t			newClientSocketAddressLen;
        int newClientFd;

        fd_set current_sockets;
        fd_set ready_sockets;
        int max_socket;
        char buff[1024];
        std::vector<client_t> myClients;
        std::vector<channel_t> myChannels;
		std::string hostname;
		std::string c_date;
};



#endif