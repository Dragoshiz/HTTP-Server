#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include <arpa/inet.h>
#include "colors.h"
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <stdexcept>
#include <sys/poll.h>
#include <map>
#include <time.h>
#include <ctime>
#include <stdlib.h>

#define REDIRECT "redirect"


#define DEBUG 0

class Client;
typedef struct sockaddr_in t_sockaddr_in;
typedef std::map <int,Client> IntClientMap;

//including multiple headers as commonheader creates additional compilation time
//in case of unnecessary or unused inclusion, compiler reads and parses all extra headers.
//Does not impact functionality or perfomance of the compiled code.
//bad practice, but I am lazy

typedef struct ErrorCheck {
	bool	code_403; // Forbidden
	bool	code_405; // Method Not Allowed
}ErrorCheck;

enum formatType{
	PLAIN, HTML, CSS, JAVASCRIPT, //text-based
	JPEG, PNG, GIF, SVGXML, //image 
	MPEG, OGGA, WAV, //audio
	MP4, OGGV, WEBM, //video
	JSON, XML, PDF, ZIP, OCTETSTREAM //application
};

int		setupPort(int listening_fd, int port, t_sockaddr_in serv_addr, int backlog);
void	printMap(IntClientMap& map);

class configParser;

void	webserv(configParser& parser);
void	setupCluster(std::vector<struct pollfd>& socketCluster, int* listening_fd, std::vector<int> ports);
void	acceptRequest(std::map<int, Client>& clients, std::vector<struct pollfd> &socketCluster, int *listening_fd, size_t ports);
void	handleRequest(std::map<int, Client>& clients, std::vector<struct pollfd> &socketCluster, configParser& parser);
void	checkPoll(std::vector<struct pollfd> &socketCluster);
void	closeClient(std::map<int, Client>& clients, std::vector<struct pollfd> &socketCluster, int index);
int		getSocketPort(int socket);
void	signalHandler(int signal);

#endif