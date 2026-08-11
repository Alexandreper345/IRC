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
#include <fcntl.h>
#include <cstring>

class Server
{
private:
	int							_port;
	std::string 				_password;
	int							_listening;
	sockaddr_in					_hint;
	int							_serverSocket;
	std::vector<pollfd> 		_fds;
	std::map<int, std::string>	_clientBuffers;

	void						initServerSocket();
	void						acceptClient();
	void						bindSocket();
	void						listenForConnections();
	void						receiveData(int fd);
	void						handlePoll();

public:
	
	Server(int port, const std::string& password);
	~Server(void);

	void						run();
};

#endif