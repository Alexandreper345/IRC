#include "Client.hpp"

Client::Client() : _fd(-1), _passOk(false), _registered(false) {}

Client::Client(int fd) : _fd(fd), _passOk(false), _registered(false) {}

int	Client::getFd() const
{
	return (_fd);
}

std::string	Client::getNickname() const
{
	return (_nickname);
}

std::string	Client::getUsername() const
{
	return (_username);
}

std::string	Client::getRealname() const
{
	return (_realname);
}

std::string	Client::getHostname() const
{
	return (_hostname);
}

bool Client::isPassOk() const
{
	return (_passOk);
}

bool Client::isRegistered() const
{
	return (_registered);
}

std::string&	Client::getInBuffer()
{
	return (_inbuffer);
}

std::string& Client::getOutBuffer()
{
	return (_outbuffer);
}
std::set<std::string> Client::getChannels() const
{
	return (_channels);
}

std::string Client::getPrefix() const
{
    return (_nickname + "!" + _username + "@" + _hostname);
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

void Client::setRealname(const std::string& realname)
{
	_realname = realname;
}

void Client::setHostname(const std::string& hostname)
{
	_hostname = hostname;
}

void Client::setPassOk(bool ok)
{
	_passOk = ok;
}

void Client::setRegistered(bool registered)
{
	_registered = registered;
}

void Client::addChannel(const std::string& channel)
{
	_channels.insert(channel);
}

void Client::removeChannel(const std::string& channel)
{
	_channels.erase(channel);
}

Client::~Client() {}