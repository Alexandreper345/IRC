/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 18:19:57 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/13 19:05:39 by anogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void) {}

Client::Client(int fd) : _fd(fd) {}

Client::Client(const Client&) {}

Client	&Client::operator=(const Client&) { return (*this); }

Client::~Client(void) {}

int                     Client::getFd(void)
{
    return (this->_fd);
}

const std::string&      Client::getNickname(void) const
{
    return (this->_nickname);
}

const std::string&      Client::getUsername(void) const
{
    return (this->_username)
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

std::set<std::string>   Client::getChannels(void) const
{
    return (this->_channels);
}

std::string&            Client::getPrefix(void) const {}

void					Client::setNickname(const std::string& nickname) {}
void					Client::setUsername(const std::string& username) {}
void					Client::setRealname(const std::string& realname) {}
void					Client::setHostname(const std::string& hostname) {}
void					Client::setPassOk(bool ok) {}
void					Client::setRegistered(bool registered) {}
void					Client::addChannel(const std::string& channel) {}
void					Client::removeChannel(const std::string& channel) {}

