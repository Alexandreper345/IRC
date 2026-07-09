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
	bool _authenticated;
	std::set<std::string> _channels;
public:
	Client();
	Client(int fd);
	~Client();
	
	int 					getFd() const;
	std::string 			getNickname() const;
	std::string 			getUsername() const;
	bool 					is_authenticated() const;
	std::set<std::string>	getChannels() const;
	
	
	void setNickname(const std::string& nickname);
	void setUsername(const std::string& username);
	void setAuthenticated(bool authenticated);
	void addChannel(const std::string& channel);
	void removeChannel(const std::string& channel);
};

#endif