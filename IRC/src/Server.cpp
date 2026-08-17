#include "Server.hpp"

Server::Server(int port, const std::string& host) : _host(host), _port(port)
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

    if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("failed to set non-blocking mode");
}

void Server::acceptClient()
{
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	int clientFd = accept(_serverSocket, (sockaddr*)&clientAddr, &clientLen);
	if (clientFd < 0)
	{
		std::cerr << "Error in accept client" << std::endl;
		return;
	}

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error setting client socket to non-blocking" << std::endl;
		close(clientFd);
		return;
	}
	pollfd p;
	p.fd = clientFd;
	p.events = POLLIN;
	p.revents = 0;
	_fds.push_back(p);
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
	char buffer[1024];
	if (fd < 0)
	{
		std::cerr << "Error fd invalid" << std::endl;
		return;
	}
	ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes == 0)
	{
		close(fd);
		_clientBuffers.erase(fd);
		for (size_t i = 0; i < _fds.size(); i++)
		{
			if(_fds[i].fd == fd)
			{
				_fds.erase(_fds.begin() + i);
				std::cout << "Client disconnected: " << fd << std::endl;
				break;
			}
		}
	}
	else if (bytes < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			std::cerr << "recv failed on fd " << fd << ": " << strerror(errno) << std::endl;
			close(fd);
			_clientBuffers.erase(fd);
			for (size_t i = 0; i < _fds.size(); i++)
			{
				if (_fds[i].fd == fd)
				{
					_fds.erase(_fds.begin() + i);
					break;
				}
			}
			
		}
		
	}
	else
	{
		_clientBuffers[fd].append(buffer, bytes);
		size_t pos = _clientBuffers[fd].find('\n');
		while (pos != std::string::npos)
		{
			std::string mensage = _clientBuffers[fd].substr(0, pos);
			_clientBuffers[fd].erase(0, pos + 1);
			std::cout << "Comando recebido de fd " << fd << ": " << mensage << std::endl;
			pos = _clientBuffers[fd].find('\n');
		}
	}
}

void Server::handlePoll()
{
	int ret = poll(_fds.data(), _fds.size(), -1);
	if (ret < 0)
		throw std::runtime_error("poll failed");
	for (size_t i = 0; i < _fds.size(); i++)
	{
    	int fd = _fds[i].fd;
    	short revents = _fds[i].revents;

    	if (revents & (POLLHUP | POLLERR))
    	{
    	    if (fd == _serverSocket)
    	        throw std::runtime_error("server socket error");
    	    receiveData(fd);
    	    continue;
    	}

    	if (revents & POLLIN)
    	{
       		if (fd == _serverSocket)
            	acceptClient();
        	else
            	receiveData(fd);
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
	for (size_t i = 0; i < _fds.size(); i++)
		close(_fds[i].fd);
}