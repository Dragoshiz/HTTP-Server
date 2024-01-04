#include "../include/configParser.hpp"
#include "../include/Client.class.hpp"
#include <cstddef>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
#include <vector>

volatile bool g_exitSignalReceived = false;

int main(int argc, char **argv)
{
	configParser parser;
	if (!parser.validConfig(argc, argv))
		return 1;
	
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	
	try {
		webserv(parser);
	}
	catch (std::exception &e)
	{
		std::cerr << BOLDRED << "Webserv error: " << e.what() << RESET_COLOR << std::endl;
		exit(1);
	}

	return 0;
}


void webserv(configParser& parser)
{
	std::vector<int>			ports = parser.getPortVector();
	std::vector<struct pollfd>	socketCluster;
	std::map <int, Client>		clients;
	int							listening_fd[ports.size()];

	setupCluster(socketCluster, listening_fd, ports);

	while(!g_exitSignalReceived)
	{
		int check = poll(socketCluster.data(), socketCluster.size(), -1);

		if (g_exitSignalReceived){
			std::cout << " Gracefully shutting down..." << std::endl;
			for (size_t i=0; i < ports.size(); i++){
				close(listening_fd[i]);
			}
			break;
		}

		if (check < 0)
			throw std::runtime_error("Error: Poll failed.");

		acceptRequest(clients, socketCluster, listening_fd, ports.size());
		
		handleRequest(clients, socketCluster, parser);
 	}
}

void checkPoll(std::vector<struct pollfd> &socketCluster)
{
	for (size_t i = 0; i < socketCluster.size(); i++)
		if (socketCluster[i].revents & POLLERR || socketCluster[i].revents & POLLHUP || socketCluster[i].revents & POLLNVAL){
			close(socketCluster[i].fd);// close socket
			socketCluster.erase(socketCluster.begin() + i);
		}
}

void setupCluster(std::vector<struct pollfd>& socketCluster, int* listening_fd, std::vector<int> ports)
{
	struct sockaddr_in	serv_addr;
	struct pollfd		tempSocket;
	int 				opt = 1;

	for (size_t index = 0; index < ports.size(); index++)
	{
		if ((listening_fd[index] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
			throw std::runtime_error("Error: Socket creation failed");

		if (setsockopt(listening_fd[index], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
			throw std::runtime_error("Error: Setting up socket option failed.");

		if (fcntl(listening_fd[index], F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1)
			throw std::runtime_error("Error: Setting up socket failed.");

		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(ports[index]);
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		if (bind(listening_fd[index], (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
			throw std::runtime_error("Error: Binding socket failed.");

		if(listen(listening_fd[index], BACKLOG) == -1)
			throw std::runtime_error("Error: Listening to socket failed.");

		tempSocket.fd = listening_fd[index];
		tempSocket.events = POLLIN | POLLOUT;
		tempSocket.revents = 0;
		socketCluster.push_back(tempSocket);
	}
}

void acceptRequest(std::map<int, Client>& clients, std::vector<struct pollfd> &socketCluster,int *listening_fd, size_t ports)
{
	sockaddr_in		addr;
	socklen_t		addr_len = sizeof(addr);
	int				newSocket;
	struct pollfd	newPoll;

	for (size_t i = 0; i < socketCluster.size(); i++)
	{ 
		if (socketCluster[i].revents & POLLIN)
		{
			if (i < ports)
			{
				if (socketCluster[i].fd == listening_fd[i])
				{
				newSocket = accept(socketCluster[i].fd, (sockaddr*)&addr, &addr_len);
				if (newSocket == -1){
					close(newSocket);
					std::cerr << "Accepting a new client failed." << std::endl;
					return;
				}
				if (fcntl(newSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1){
					close(newSocket);
					std::cerr << "Setting up a new client failed." << std::endl;
					return;
				}
				newPoll.fd		=	newSocket;
				newPoll.events	=	POLLIN;
				newPoll.revents =	0;
	
				Client newClient(newSocket, getSocketPort(socketCluster[i].fd));
				socketCluster.push_back(newPoll);
				clients.insert(std::pair<int, Client>(newSocket, newClient));
				}
			}
		}
	}
}

void handleRequest(std::map<int, Client>& clients, std::vector<struct pollfd> &socketCluster, configParser& parser)
{
	int check;
	for (size_t i = 0; i < socketCluster.size(); i++) {
		IntClientMap::iterator client = clients.find(socketCluster[i].fd);

		if (client == clients.end())
			continue;

		if (socketCluster[i].revents & POLLIN && socketCluster[i].fd == client->second.getFd()){

			check = client->second.readData();

			if (check < 0){
				std::cerr << BOLDRED << socketCluster[i].fd << " failed " << RESET_COLOR << std::endl;
				closeClient(clients, socketCluster, i);
				continue;
			}
			else if (check == 0){
				closeClient(clients, socketCluster, i);
			}
			else{
				if (client->second.processRequest(parser) == 0)
					continue;

				socketCluster[i].events = POLLOUT;
			}
		}
		else if (socketCluster[i].revents & POLLOUT && socketCluster[i].fd == client->second.getFd())
		{
			check = client->second.sendData();

			if (check == 0){
				socketCluster[i].events = 0;
				closeClient(clients, socketCluster, i);
				continue;
			}
			else if(check < 0){
				closeClient(clients, socketCluster, i);
				continue;
			}
			else if (client->second.getBytesRemaining() > 0){

				if(client->second.readFile(client->second.getRequestedFile()) == 0)
					closeClient(clients, socketCluster, i);
				continue;
			}
			if (check < CHUNK_SIZE)
				closeClient(clients, socketCluster, i);
		}
	}
}


void closeClient(std::map<int, Client>& clients, std::vector<struct pollfd> &socketCluster, int index){
	close(socketCluster[index].fd);
	clients.erase(socketCluster[index].fd);
	socketCluster.erase(socketCluster.begin() + index);
}
int getSocketPort(int socket)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int port = 0;

	if (getsockname(socket, (struct sockaddr *)&addr, &len) == -1)
		return -1;
	port = ntohs(addr.sin_port);
	return port;
}

void signalHandler(int signal){
	if (signal == SIGINT || signal == SIGTERM){
		g_exitSignalReceived = true;
	}
}