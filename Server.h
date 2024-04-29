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
    std::string hostname;
    std::string password;
    bool        is_auth;
}client_t;

typedef struct channel_t
{
    std::string name;
    std::string password;
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
        void acceptNewClient();
        void parseClient(int i);
        void parseLine(int i);
        int createSocket();
        int setSocketOpt();
        int setNonBlock();
        int bindSocket();
        int listenSocket();
        int acceptSocket();

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
};



#endif