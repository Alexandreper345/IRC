/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 18:19:57 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/20 16:18:20 by anogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Client.hpp"

Client::Client(void) : _fd(-1), _passOk(false), _registered(false) {}

Client::Client(int fd) : _fd(fd) {}

Client::Client(const Client& other)
{
    _fd = other._fd;
    _nickname = other._nickname;
    _username = other._username;
    _realname = other._realname;
    _hostname = other._hostname;
    _inbuffer = other._inbuffer;
    _outbuffer = other._outbuffer;
    _passOk = other._passOk;
    _registered = other._registered;
    _channels = other._channels;
}

Client	&Client::operator=(const Client& other)
{ 
    if (this != &other)
    {
        _fd = other._fd;
        _nickname = other._nickname;
        _username = other._username;
        _realname = other._realname;
        _hostname = other._hostname;
        _inbuffer = other._inbuffer;
        _outbuffer = other._outbuffer;
        _passOk = other._passOk;
        _registered = other._registered;
        _channels = other._channels;
    }
    return (*this);
}

Client::~Client(void) {}

int                     Client::getFd(void) const
{
    return (this->_fd);
}

const std::string&      Client::getNickname(void) const
{
    return (this->_nickname);
}

const std::string&      Client::getUsername(void) const
{
    return (this->_username);
}

const std::string&      Client::getRealname(void) const
{
    return (this->_realname);
}

const std::string&      Client::getHostname(void) const
{
    return (this->_hostname);
}

bool                    Client::isPassOk(void) const
{
    return (this->_passOk);
}

bool                    Client::isRegistered(void) const
{
    return (this->_registered);
}

std::string&            Client::getInBuffer(void)
{
    return (this->_inbuffer);
}

std::string&            Client::getOutBuffer(void)
{
    return (this->_outbuffer);
}

const std::set<std::string>&   Client::getChannels(void) const
{
    return (this->_channels);
}

std::string            Client::getPrefix(void) const
{
    std::string prefix = ":" + _nickname + "!" + _username + "@" + _hostname;
    return (prefix);
}

void					Client::setNickname(const std::string& nickname)
{
    _nickname = nickname;
}
void					Client::setUsername(const std::string& username)
{
    _username = username;
}
void					Client::setRealname(const std::string& realname)
{
    _realname = realname;
}
void					Client::setHostname(const std::string& hostname)
{
    _hostname = hostname;
}
void					Client::setPassOk(bool ok)
{
    _passOk = ok;
}
void					Client::setRegistered(bool registered)
{
    _registered = registered;
}
void					Client::addChannel(const std::string& channel)
{
    _channels.insert(channel);
}
void					Client::removeChannel(const std::string& channel)
{
    _channels.erase(channel);
}

