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
	std::map<int, std::string>	_clientBuffers;

	void						initServerSocket(void);
	void						acceptClient(void);
	void						bindSocket(void);
	void						listenForConnections(void);
	void						receiveData(int fd);
	void						removeClient(int fd);
	void						handlePoll(void);

public:
	
	Server(int port, const std::string& password);
	~Server(void);

	void						run(void);

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
};

#endif
