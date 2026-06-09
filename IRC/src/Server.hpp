#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdexcept>

class Server
{
private:
	int _port;
	std::string _host;
	int _serverSocket;
	std::vector<pollfd> _fds;

	void initServerSocket();
	void acceptClient();
	void bindSocket();
	void listenForConnections();
	void receiveData(int fd);
	void handlePoll();

public:
	Server(int port, const std::string& host);
	~Server();

	void run();
};

#endif