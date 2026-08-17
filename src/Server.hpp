#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <poll.h>
# include <stdexcept>
# include <fcntl.h>
# include <cstring>
# include <cerrno>
# include <signal.h>

# include "Client.hpp"

# define MAX_CLIENTS 1024
# define RECV_BUFFER_SIZE 1024

class Server
{
private:
	int							_port;
	std::string 				_password;
	int							_socket;
	sockaddr_in					_hint;
	std::vector<pollfd> 		_fds;
	std::map<int, Client>		_clients;

	void						initServerSocket(void);
	void						acceptClient(void);
	void						bindSocket(void);
	void						listenForConnections(void);
	void						receiveData(int fd);
	void						removeClient(int fd);
	void						handlePoll(void);
	void    					setupSignals(void);

public:
	
	Server(void);
	Server(const Server&);
	Server	&operator=(const Server&);
	Server(int port, const std::string& password);
	~Server(void);

	void						run(void);
	void						sendData(Client &client);

	class ServerSocketError : public std::exception {
		public:
			virtual const char	*what() const throw();
	};

	class SetNonBlockError : public std::exception {
		public:
			virtual const char	*what() const throw();
	};

	class BindPortError : public std::exception {
		public:
			virtual const char	*what() const throw();
	};

	class PollError : public std::exception {
		public:
			virtual const char	*what() const throw();
	};
	
	class ListeningError : public std::exception {
		public:
			virtual const char	*what() const throw();
	};

	class SignalSetupError : public std::exception {
		public:
			virtual const char	*what() const throw();
	};
};

#endif
