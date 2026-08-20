/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 17:33:12 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/18 12:41:00 by anogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <set>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <poll.h>
# include <stdexcept>
# include <fcntl.h>
# include <cstring>
# include <cerrno>
# include <signal.h>
# include <arpa/inet.h>
# include <cctype>

# include "Client.hpp"

# define MAX_CLIENTS 1024
# define RECV_BUFFER_SIZE 1024
# define MAX_BUFFER_SIZE 4096
# define MAX_MSG_SIZE 510
# define SERVER_NAME "ircserv"

typedef struct s_message
{
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
}				t_message;

class Server
{
private:
	int							_port;
	std::string 				_password;
	int							_socket;
	sockaddr_in					_hint;
	std::vector<pollfd> 		_fds;
	std::map<int, Client>		_clients;
	t_message					_message;

	void						initServerSocket(void);
	void						acceptClient(void);
	void						bindSocket(void);
	void						listenForConnections(void);
	void						receiveData(int fd);
	void						sendData(int fd);
	void						removeClient(int fd);
	void						handlePoll(void);
	void						setupSignals(void);
	void						parseLine(int fd, std::string line);
	void						extractLines(int fd);
	void						handleCommand(int fd);

	void						queueMessage(int fd, const std::string& message);
	void						numericReply(int fd, const std::string& code,
									const std::string& args, const std::string& text);
	void						setPollOut(int fd, bool enable);
	Client*						getClient(int fd);
	int							findClientByNick(const std::string& nick);
	bool						isValidNick(const std::string& nick);
	bool						isValidChannel(const std::string& name);
	bool						isInChannel(int fd, const std::string& channel);
	void						broadcastToChannel(const std::string& channel,
									const std::string& message, int exceptFd);
	void						tryRegister(int fd);
	void						sendNames(int fd, const std::string& channel);
	std::vector<std::string>	splitComma(const std::string& s);

	void						cmdPass(int fd);
	void						cmdNick(int fd);
	void						cmdUser(int fd);
	void						cmdJoin(int fd);
	void						cmdKick(int fd);
	void						cmdInvite(int fd);
	void						cmdTopic(int fd);
	void						cmdMode(int fd);
	void						cmdPart(int fd);
	void						cmdQuit(int fd);
	void						cmdPrivmsg(int fd);
	void						cmdPing(int fd);

public:

	Server(void);
	Server(const Server&);
	Server	&operator=(const Server&);
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

	class SignalSetupError : public std::exception {
		public:
			virtual const char	*what() const throw();
	};
};

#endif
