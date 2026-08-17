#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>


class Client
{
	private:
		int						_fd;
		std::string 			_nickname;
		std::string 			_username;
		std::string 			_realname;
		std::string 			_hostname;
		std::string 			_inbuffer;
		std::string 			_outbuffer;
		bool 					_passOk;
		bool 					_registered;
		std::set<std::string>	_channels;
	public:
		Client(void);
		Client(int _fd);
		Client(const Client& other);
		Client	&operator=(const Client& other);
		~Client(void);

		int                     getFd(void) const;
		const std::string&      getNickname(void) const;
		const std::string&      getUsername(void) const;
		const std::string&      getRealname(void) const;
		const std::string&      getHostname(void) const;
		
		bool                    isPassOk(void) const;
		bool                    isRegistered(void) const;
		
		std::string&            getInBuffer(void);
		std::string&            getOutBuffer(void);
		const std::set<std::string>&   getChannels(void) const;
		std::string            getPrefix(void) const;
		
		void					setNickname(const std::string& nickname);
		void					setUsername(const std::string& username);
		void					setRealname(const std::string& realname);
		void					setHostname(const std::string& hostname);
		void					setPassOk(bool ok);
		void					setRegistered(bool registered);
		void					addChannel(const std::string& channel);
		void					removeChannel(const std::string& channel);
};

#endif