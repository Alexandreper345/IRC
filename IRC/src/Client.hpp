#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>


class Client
{
	private:
		int _fd;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _hostname;
		std::string _inbuffer;
		std::string _outbuffer;
		bool _passOk;
		bool _registered;
		std::set<std::string> _channels;
	public:
		Client();
		Client(int fd);
		~Client();
		int                     getFd() const;
		std::string             getNickname() const;
		std::string             getUsername() const;
		std::string             getRealname() const;
		std::string             getHostname() const;
		bool                    isPassOk() const;
		bool                    isRegistered() const;
		std::string&            getInBuffer();
		std::string&            getOutBuffer();
		std::set<std::string>   getChannels() const;
		std::string             getPrefix() const;
		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setRealname(const std::string& realname);
		void setHostname(const std::string& hostname);
		void setPassOk(bool ok);
		void setRegistered(bool registered);
		void addChannel(const std::string& channel);
		void removeChannel(const std::string& channel);
};

#endif