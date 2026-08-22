/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 17:00:00 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/20 17:00:00 by anogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Channel.hpp"
#include <sstream>

Channel::Channel(void)
	: _inviteOnly(false), _topicRestricted(true), _userLimit(0) {}

Channel::Channel(const std::string& name)
	: _name(name), _inviteOnly(false), _topicRestricted(true), _userLimit(0) {}

Channel::Channel(const Channel& other)
{
	*this = other;
}

Channel	&Channel::operator=(const Channel& other)
{
	if (this != &other)
	{
		_name = other._name;
		_topic = other._topic;
		_key = other._key;
		_members = other._members;
		_operators = other._operators;
		_invited = other._invited;
		_inviteOnly = other._inviteOnly;
		_topicRestricted = other._topicRestricted;
		_userLimit = other._userLimit;
	}
	return (*this);
}

Channel::~Channel(void) {}

const std::string&	Channel::getName(void) const
{
	return (_name);
}

const std::string&	Channel::getTopic(void) const
{
	return (_topic);
}

void	Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

const std::string&	Channel::getKey(void) const
{
	return (_key);
}

void	Channel::setKey(const std::string& key)
{
	_key = key;
}

void	Channel::unsetKey(void)
{
	_key.clear();
}

bool	Channel::hasKey(void) const
{
	return (!_key.empty());
}

void	Channel::addMember(int fd)
{
	_members.insert(fd);
	_invited.erase(fd);
}

void	Channel::removeMember(int fd)
{
	_members.erase(fd);
	_operators.erase(fd);
	_invited.erase(fd);
}

bool	Channel::hasMember(int fd) const
{
	return (_members.find(fd) != _members.end());
}

const std::set<int>&	Channel::getMembers(void) const
{
	return (_members);
}

bool	Channel::isEmpty(void) const
{
	return (_members.empty());
}

size_t	Channel::memberCount(void) const
{
	return (_members.size());
}

void	Channel::addOperator(int fd)
{
	if (hasMember(fd))
		_operators.insert(fd);
}

void	Channel::removeOperator(int fd)
{
	_operators.erase(fd);
}

bool	Channel::isOperator(int fd) const
{
	return (_operators.find(fd) != _operators.end());
}

void	Channel::invite(int fd)
{
	_invited.insert(fd);
}

void	Channel::removeInvite(int fd)
{
	_invited.erase(fd);
}

bool	Channel::isInvited(int fd) const
{
	return (_invited.find(fd) != _invited.end());
}

bool	Channel::isInviteOnly(void) const
{
	return (_inviteOnly);
}

void	Channel::setInviteOnly(bool enable)
{
	_inviteOnly = enable;
}

bool	Channel::isTopicRestricted(void) const
{
	return (_topicRestricted);
}

void	Channel::setTopicRestricted(bool enable)
{
	_topicRestricted = enable;
}

size_t	Channel::getUserLimit(void) const
{
	return (_userLimit);
}

void	Channel::setUserLimit(size_t limit)
{
	_userLimit = limit;
}

std::string	Channel::getModeString(void) const
{
	std::string			modes;
	std::string			args;
	std::ostringstream	oss;

	modes = "+";
	if (_inviteOnly)
		modes += "i";
	if (_topicRestricted)
		modes += "t";
	if (!_key.empty())
	{
		modes += "k";
		args += " " + _key;
	}
	if (_userLimit > 0)
	{
		modes += "l";
		oss << _userLimit;
		args += " " + oss.str();
	}
	return (modes + args);
}
