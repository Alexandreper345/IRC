#include "Server.hpp"

Server::Server(int port, const std::string& host)
{
}

void Server::initServerSocket()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
		throw std::runtime_error("failed to create socket");

	int enable = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
		throw std::runtime_error("failed to set socket options");
}

void Server::acceptClient()
{

}

void Server::bindSocket()
{
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(_port);
	if (bind(_serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		throw std::runtime_error("failed to bind socket");
}

void Server::listenForConnections()
{
	pollfd p;

	if (listen(_serverSocket, 5) < 0)
		throw std::runtime_error("failed to listen for connections");


	p.fd = _serverSocket;
	p.events = POLLIN;
	p.revents = 0;

	_fds.push_back(p);
}

void Server::receiveData(int fd)
{

}

void Server::handlePoll()
{
	int ret = poll(_fds.data(), _fds.size(), -1);
	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd == _serverSocket )
		{
			acceptClient();
		}
		else
		{
			receiveData(_fds[i].fd);
		}
	}
	
}


void Server::run()
{
	initServerSocket();
	bindSocket();
	listenForConnections();

	while (true)
	{
		handlePoll();
	}
}



Server::~Server()
{
}