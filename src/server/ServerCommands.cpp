/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCommands.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 12:41:00 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/18 12:41:00 by anogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Server.hpp"
#include <cstdlib>

static std::string toUpper(std::string s)
{
    for (size_t i = 0; i < s.size(); i++)
        s[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[i])));
    return (s);
}

Client  *Server::getClient(int fd)
{
    std::map<int, Client>::iterator it;

    it = _clients.find(fd);
    if (it == _clients.end())
        return (NULL);
    return (&it->second);
}

Channel *Server::getChannel(const std::string& name)
{
    std::map<std::string, Channel>::iterator it;

    it = _channels.find(name);
    if (it == _channels.end())
        return (NULL);
    return (&it->second);
}

void    Server::removeClientFromChannel(int fd, const std::string& name)
{
    Client  *client;
    Channel *channel;

    client = getClient(fd);
    if (client)
        client->removeChannel(name);
    channel = getChannel(name);
    if (!channel)
        return ;
    channel->removeMember(fd);
    if (channel->isEmpty())
        _channels.erase(name);
}

void    Server::partClientFromChannels(int fd, const std::string& quitMsg)
{
    Client                              *client;
    Channel                             *channel;
    std::set<std::string>               chans;
    std::set<std::string>::const_iterator it;
    std::string                         msg;

    client = getClient(fd);
    if (!client)
        return ;
    chans = client->getChannels();
    if (chans.empty())
        return ;
    msg = quitMsg;
    if (msg.empty())
        msg = client->getPrefix() + " QUIT :Connection closed";
    for (it = chans.begin(); it != chans.end(); ++it)
    {
        broadcastToChannel(*it, msg, fd);
        channel = getChannel(*it);
        if (channel)
        {
            channel->removeMember(fd);
            if (channel->isEmpty())
                _channels.erase(*it);
        }
        client->removeChannel(*it);
    }
}

int Server::findClientByNick(const std::string& nick)
{
    std::map<int, Client>::iterator it;
    std::string                     target;

    target = toUpper(nick);
    for (it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (toUpper(it->second.getNickname()) == target)
            return (it->first);
    }
    return (-1);
}

bool    Server::isValidNick(const std::string& nick)
{
    unsigned char   c;

    if (nick.empty() || nick.size() > 16)
        return (false);
    c = static_cast<unsigned char>(nick[0]);
    if (!std::isalpha(c) && std::string("[]\\`_^{|}").find(nick[0]) == std::string::npos)
        return (false);
    for (size_t i = 1; i < nick.size(); i++)
    {
        c = static_cast<unsigned char>(nick[i]);
        if (!std::isalnum(c) && std::string("[]\\`_^{|}-").find(nick[i]) == std::string::npos)
            return (false);
    }
    return (true);
}

bool    Server::isValidChannel(const std::string& name)
{
    if (name.size() < 2 || name.size() > 50 || name[0] != '#')
        return (false);
    for (size_t i = 1; i < name.size(); i++)
    {
        if (name[i] == ' ' || name[i] == ',' || name[i] == 7)
            return (false);
    }
    return (true);
}

bool    Server::isInChannel(int fd, const std::string& channel)
{
    Channel *ch;

    ch = getChannel(channel);
    if (ch)
        return (ch->hasMember(fd));
    return (false);
}

std::vector<std::string>    Server::splitComma(const std::string& s)
{
    std::vector<std::string>    out;
    std::string                 item;

    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == ',')
        {
            if (!item.empty())
                out.push_back(item);
            item.clear();
        }
        else
            item += s[i];
    }
    if (!item.empty())
        out.push_back(item);
    return (out);
}

void    Server::queueMessage(int fd, const std::string& message)
{
    Client  *client;

    client = getClient(fd);
    if (!client)
        return ;
    client->getOutBuffer() += message + "\r\n";
    setPollOut(fd, true);
}

void    Server::numericReply(int fd, const std::string& code,
    const std::string& args, const std::string& text)
{
    Client      *client;
    std::string nick;
    std::string msg;

    client = getClient(fd);
    if (!client)
        return ;
    nick = client->getNickname();
    if (nick.empty())
        nick = "*";
    msg = ":";
    msg += SERVER_NAME;
    msg += " ";
    msg += code;
    msg += " ";
    msg += nick;
    if (!args.empty())
    {
        msg += " ";
        msg += args;
    }
    if (!text.empty())
    {
        msg += " :";
        msg += text;
    }
    queueMessage(fd, msg);
}

void    Server::broadcastToChannel(const std::string& channel,
    const std::string& message, int exceptFd)
{
    Channel                     *ch;
    std::set<int>::const_iterator it;

    ch = getChannel(channel);
    if (!ch)
        return ;
    for (it = ch->getMembers().begin(); it != ch->getMembers().end(); ++it)
    {
        if (*it != exceptFd)
            queueMessage(*it, message);
    }
}

void    Server::sendNames(int fd, const std::string& channel)
{
    Client                          *client;
    Client                          *member;
    Channel                         *ch;
    std::set<int>::const_iterator   it;
    std::string                     names;
    std::string                     nick;
    std::string                     entry;

    client = getClient(fd);
    ch = getChannel(channel);
    if (!client || !ch)
        return ;
    nick = client->getNickname();
    for (it = ch->getMembers().begin(); it != ch->getMembers().end(); ++it)
    {
        member = getClient(*it);
        if (!member || member->getNickname().empty())
            continue ;
        if (!names.empty())
            names += " ";
        entry.clear();
        if (ch->isOperator(*it))
            entry += "@";
        entry += member->getNickname();
        names += entry;
    }
    queueMessage(fd, std::string(":") + SERVER_NAME + " 353 " + nick
        + " = " + channel + " :" + names);
    numericReply(fd, "366", channel, "End of /NAMES list");
}

void    Server::tryRegister(int fd)
{
    Client  *client;

    client = getClient(fd);
    if (!client || client->isRegistered())
        return ;
    if (!client->isPassOk() || client->getNickname().empty()
        || client->getUsername().empty())
        return ;
    client->setRegistered(true);
    numericReply(fd, "001", "", "Welcome to the Internet Relay Network "
        + client->getPrefix().substr(1));
    numericReply(fd, "002", "",
        std::string("Your host is ") + SERVER_NAME + ", running version 1.0");
    numericReply(fd, "003", "", "This server was created 2026");
    queueMessage(fd, std::string(":") + SERVER_NAME + " 004 "
        + client->getNickname() + " " + SERVER_NAME + " 1.0 o itkol");
}

void    Server::cmdPass(int fd)
{
    Client  *client;

    client = getClient(fd);
    if (!client)
        return ;
    if (client->isRegistered())
    {
        numericReply(fd, "462", "", "You may not reregister");
        return ;
    }
    if (_message.params.empty())
    {
        numericReply(fd, "461", "PASS", "Not enough parameters");
        return ;
    }
    if (_message.params[0] != _password)
    {
        client->setPassOk(false);
        numericReply(fd, "464", "", "Password incorrect");
        return ;
    }
    client->setPassOk(true);
    tryRegister(fd);
}

void    Server::cmdNick(int fd)
{
    Client      *client;
    std::string oldNick;
    std::string newNick;
    int         other;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty() || _message.params[0].empty())
    {
        numericReply(fd, "431", "", "No nickname given");
        return ;
    }
    newNick = _message.params[0];
    if (!isValidNick(newNick))
    {
        numericReply(fd, "432", newNick, "Erroneous nickname");
        return ;
    }
    other = findClientByNick(newNick);
    if (other != -1 && other != fd)
    {
        numericReply(fd, "433", newNick, "Nickname is already in use");
        return ;
    }
    oldNick = client->getNickname();
    if (toUpper(oldNick) == toUpper(newNick))
        return ;
    client->setNickname(newNick);
    if (client->isRegistered())
    {
        std::set<std::string>                       chans;
        std::set<std::string>::const_iterator       it;
        std::string                                 msg;
        std::map<int, bool>                         seen;
        std::map<int, Client>::iterator             cit;

        msg = ":" + oldNick + "!" + client->getUsername() + "@"
            + client->getHostname() + " NICK :" + newNick;
        queueMessage(fd, msg);
        chans = client->getChannels();
        for (it = chans.begin(); it != chans.end(); ++it)
        {
            for (cit = _clients.begin(); cit != _clients.end(); ++cit)
            {
                if (cit->first != fd && isInChannel(cit->first, *it)
                    && seen.find(cit->first) == seen.end())
                {
                    queueMessage(cit->first, msg);
                    seen[cit->first] = true;
                }
            }
        }
    }
    tryRegister(fd);
}

void    Server::cmdUser(int fd)
{
    Client  *client;

    client = getClient(fd);
    if (!client)
        return ;
    if (client->isRegistered())
    {
        numericReply(fd, "462", "", "You may not reregister");
        return ;
    }
    if (_message.params.size() < 4)
    {
        numericReply(fd, "461", "USER", "Not enough parameters");
        return ;
    }
    client->setUsername(_message.params[0]);
    client->setRealname(_message.params[3]);
    tryRegister(fd);
}

void    Server::cmdPing(int fd)
{
    if (_message.params.empty())
    {
        numericReply(fd, "409", "", "No origin specified");
        return ;
    }
    queueMessage(fd, std::string(":") + SERVER_NAME + " PONG " + SERVER_NAME
        + " :" + _message.params[0]);
}

void    Server::cmdQuit(int fd)
{
    Client      *client;
    std::string reason;
    std::string msg;

    client = getClient(fd);
    if (!client)
        return ;
    reason = _message.params.empty() ? "Client Quit" : _message.params[0];
    msg = client->getPrefix() + " QUIT :" + reason;
    partClientFromChannels(fd, msg);
    removeClient(fd);
}

void    Server::cmdJoin(int fd)
{
    Client                      *client;
    Channel                     *channel;
    std::vector<std::string>    channels;
    std::vector<std::string>    keys;
    std::string                 joinMsg;
    std::string                 key;
    bool                        created;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty() || _message.params[0].empty())
    {
        numericReply(fd, "461", "JOIN", "Not enough parameters");
        return ;
    }
    channels = splitComma(_message.params[0]);
    if (_message.params.size() > 1)
        keys = splitComma(_message.params[1]);
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (!isValidChannel(channels[i]))
        {
            numericReply(fd, "403", channels[i], "No such channel");
            continue ;
        }
        if (isInChannel(fd, channels[i]))
            continue ;
        key = (i < keys.size()) ? keys[i] : "";
        channel = getChannel(channels[i]);
        created = false;
        if (!channel)
        {
            _channels.insert(std::make_pair(channels[i], Channel(channels[i])));
            channel = getChannel(channels[i]);
            created = true;
        }
        if (!channel)
            continue ;
        if (!created && channel->isInviteOnly() && !channel->isInvited(fd))
        {
            numericReply(fd, "473", channels[i], "Cannot join channel (+i)");
            continue ;
        }
        if (!created && channel->hasKey() && channel->getKey() != key)
        {
            numericReply(fd, "475", channels[i], "Cannot join channel (+k)");
            continue ;
        }
        if (!created && channel->getUserLimit() > 0
            && channel->memberCount() >= channel->getUserLimit())
        {
            numericReply(fd, "471", channels[i], "Cannot join channel (+l)");
            continue ;
        }
        channel->addMember(fd);
        if (created)
            channel->addOperator(fd);
        client->addChannel(channels[i]);
        joinMsg = client->getPrefix() + " JOIN :" + channels[i];
        queueMessage(fd, joinMsg);
        broadcastToChannel(channels[i], joinMsg, fd);
        if (channel->getTopic().empty())
            numericReply(fd, "331", channels[i], "No topic is set");
        else
            numericReply(fd, "332", channels[i], channel->getTopic());
        sendNames(fd, channels[i]);
    }
}

void    Server::cmdPart(int fd)
{
    Client                      *client;
    std::vector<std::string>    channels;
    std::string                 reason;
    std::string                 partMsg;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty() || _message.params[0].empty())
    {
        numericReply(fd, "461", "PART", "Not enough parameters");
        return ;
    }
    reason = (_message.params.size() > 1) ? _message.params[1] : client->getNickname();
    channels = splitComma(_message.params[0]);
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (!getChannel(channels[i]))
        {
            numericReply(fd, "403", channels[i], "No such channel");
            continue ;
        }
        if (!isInChannel(fd, channels[i]))
        {
            numericReply(fd, "442", channels[i], "You're not on that channel");
            continue ;
        }
        partMsg = client->getPrefix() + " PART " + channels[i] + " :" + reason;
        queueMessage(fd, partMsg);
        broadcastToChannel(channels[i], partMsg, fd);
        removeClientFromChannel(fd, channels[i]);
    }
}

void    Server::cmdPrivmsg(int fd)
{
    Client      *client;
    std::string target;
    std::string text;
    std::string msg;
    int         dest;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty() || _message.params[0].empty())
    {
        numericReply(fd, "411", "", "No recipient given (PRIVMSG)");
        return ;
    }
    if (_message.params.size() < 2 || _message.params[1].empty())
    {
        numericReply(fd, "412", "", "No text to send");
        return ;
    }
    target = _message.params[0];
    text = _message.params[1];
    msg = client->getPrefix() + " PRIVMSG " + target + " :" + text;
    if (target[0] == '#')
    {
        if (!isInChannel(fd, target))
        {
            numericReply(fd, "404", target, "Cannot send to channel");
            return ;
        }
        broadcastToChannel(target, msg, fd);
        return ;
    }
    dest = findClientByNick(target);
    if (dest == -1)
    {
        numericReply(fd, "401", target, "No such nick/channel");
        return ;
    }
    queueMessage(dest, msg);
}

void    Server::cmdKick(int fd)
{
    Client      *client;
    Client      *target;
    Channel     *channel;
    std::string channelName;
    std::string nick;
    std::string reason;
    std::string kickMsg;
    int         targetFd;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.size() < 2)
    {
        numericReply(fd, "461", "KICK", "Not enough parameters");
        return ;
    }
    channelName = _message.params[0];
    nick = _message.params[1];
    reason = (_message.params.size() > 2) ? _message.params[2] : client->getNickname();
    channel = getChannel(channelName);
    if (!channel)
    {
        numericReply(fd, "403", channelName, "No such channel");
        return ;
    }
    if (!channel->hasMember(fd))
    {
        numericReply(fd, "442", channelName, "You're not on that channel");
        return ;
    }
    if (!channel->isOperator(fd))
    {
        numericReply(fd, "482", channelName, "You're not channel operator");
        return ;
    }
    targetFd = findClientByNick(nick);
    if (targetFd == -1)
    {
        numericReply(fd, "401", nick, "No such nick/channel");
        return ;
    }
    if (!channel->hasMember(targetFd))
    {
        numericReply(fd, "441", nick + " " + channelName, "They aren't on that channel");
        return ;
    }
    target = getClient(targetFd);
    kickMsg = client->getPrefix() + " KICK " + channelName + " " + nick + " :" + reason;
    queueMessage(fd, kickMsg);
    broadcastToChannel(channelName, kickMsg, fd);
    if (target)
        removeClientFromChannel(targetFd, channelName);
}

void    Server::cmdInvite(int fd)
{
    Client      *client;
    Channel     *channel;
    std::string nick;
    std::string channelName;
    int         targetFd;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.size() < 2)
    {
        numericReply(fd, "461", "INVITE", "Not enough parameters");
        return ;
    }
    nick = _message.params[0];
    channelName = _message.params[1];
    channel = getChannel(channelName);
    if (!channel)
    {
        numericReply(fd, "403", channelName, "No such channel");
        return ;
    }
    if (!channel->hasMember(fd))
    {
        numericReply(fd, "442", channelName, "You're not on that channel");
        return ;
    }
    if (!channel->isOperator(fd))
    {
        numericReply(fd, "482", channelName, "You're not channel operator");
        return ;
    }
    targetFd = findClientByNick(nick);
    if (targetFd == -1)
    {
        numericReply(fd, "401", nick, "No such nick/channel");
        return ;
    }
    if (channel->hasMember(targetFd))
    {
        numericReply(fd, "443", nick + " " + channelName, "is already on channel");
        return ;
    }
    channel->invite(targetFd);
    numericReply(fd, "341", nick + " " + channelName, "");
    queueMessage(targetFd, client->getPrefix() + " INVITE " + nick + " :" + channelName);
}

void    Server::cmdTopic(int fd)
{
    Client      *client;
    Channel     *channel;
    std::string channelName;
    std::string topicMsg;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty())
    {
        numericReply(fd, "461", "TOPIC", "Not enough parameters");
        return ;
    }
    channelName = _message.params[0];
    channel = getChannel(channelName);
    if (!channel)
    {
        numericReply(fd, "403", channelName, "No such channel");
        return ;
    }
    if (!channel->hasMember(fd))
    {
        numericReply(fd, "442", channelName, "You're not on that channel");
        return ;
    }
    if (_message.params.size() == 1)
    {
        if (channel->getTopic().empty())
            numericReply(fd, "331", channelName, "No topic is set");
        else
            numericReply(fd, "332", channelName, channel->getTopic());
        return ;
    }
    if (channel->isTopicRestricted() && !channel->isOperator(fd))
    {
        numericReply(fd, "482", channelName, "You're not channel operator");
        return ;
    }
    channel->setTopic(_message.params[1]);
    topicMsg = client->getPrefix() + " TOPIC " + channelName + " :" + _message.params[1];
    queueMessage(fd, topicMsg);
    broadcastToChannel(channelName, topicMsg, fd);
}

void    Server::cmdMode(int fd)
{
    Client      *client;
    Channel     *channel;
    Client      *target;
    std::string targetName;
    std::string modes;
    std::string applied;
    std::string appliedArgs;
    std::string arg;
    std::string modeMsg;
    char        sign;
    char        lastSign;
    size_t      argi;
    int         targetFd;
    long        limit;
    char        *end;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty())
    {
        numericReply(fd, "461", "MODE", "Not enough parameters");
        return ;
    }
    targetName = _message.params[0];
    if (targetName[0] != '#')
        return ;
    channel = getChannel(targetName);
    if (!channel)
    {
        numericReply(fd, "403", targetName, "No such channel");
        return ;
    }
    if (!channel->hasMember(fd))
    {
        numericReply(fd, "442", targetName, "You're not on that channel");
        return ;
    }
    if (_message.params.size() == 1)
    {
        queueMessage(fd, std::string(":") + SERVER_NAME + " 324 "
            + client->getNickname() + " " + targetName + " "
            + channel->getModeString());
        return ;
    }
    if (!channel->isOperator(fd))
    {
        numericReply(fd, "482", targetName, "You're not channel operator");
        return ;
    }
    sign = 0;
    lastSign = 0;
    argi = 1;
    while (argi < _message.params.size())
    {
        modes = _message.params[argi];
        if (modes.empty() || (modes[0] != '+' && modes[0] != '-'))
            break ;
        argi++;
        for (size_t i = 0; i < modes.size(); i++)
        {
            if (modes[i] == '+' || modes[i] == '-')
            {
                sign = modes[i];
                continue ;
            }
            if (sign == 0)
                continue ;
            arg.clear();
            if (modes[i] == 'i')
                channel->setInviteOnly(sign == '+');
            else if (modes[i] == 't')
                channel->setTopicRestricted(sign == '+');
            else if (modes[i] == 'k')
            {
                if (sign == '+')
                {
                    if (argi >= _message.params.size() || _message.params[argi].empty())
                    {
                        numericReply(fd, "461", "MODE", "Not enough parameters");
                        continue ;
                    }
                    arg = _message.params[argi++];
                    channel->setKey(arg);
                }
                else
                    channel->unsetKey();
            }
            else if (modes[i] == 'l')
            {
                if (sign == '+')
                {
                    if (argi >= _message.params.size())
                    {
                        numericReply(fd, "461", "MODE", "Not enough parameters");
                        continue ;
                    }
                    arg = _message.params[argi++];
                    limit = std::strtol(arg.c_str(), &end, 10);
                    if (*end != '\0' || limit <= 0 || limit > MAX_CLIENTS)
                        continue ;
                    channel->setUserLimit(static_cast<size_t>(limit));
                }
                else
                    channel->setUserLimit(0);
            }
            else if (modes[i] == 'o')
            {
                if (argi >= _message.params.size() || _message.params[argi].empty())
                {
                    numericReply(fd, "461", "MODE", "Not enough parameters");
                    continue ;
                }
                arg = _message.params[argi++];
                targetFd = findClientByNick(arg);
                target = (targetFd == -1) ? NULL : getClient(targetFd);
                if (!target)
                {
                    numericReply(fd, "401", arg, "No such nick/channel");
                    continue ;
                }
                if (!channel->hasMember(targetFd))
                {
                    numericReply(fd, "441", arg + " " + targetName,
                        "They aren't on that channel");
                    continue ;
                }
                if (sign == '+')
                    channel->addOperator(targetFd);
                else
                    channel->removeOperator(targetFd);
            }
            else
            {
                numericReply(fd, "472", std::string(1, modes[i]),
                    "is unknown mode char to me");
                continue ;
            }
            if (sign != lastSign)
            {
                applied += sign;
                lastSign = sign;
            }
            applied += modes[i];
            if (!arg.empty())
                appliedArgs += " " + arg;
        }
    }
    if (applied.empty())
        return ;
    modeMsg = client->getPrefix() + " MODE " + targetName + " " + applied + appliedArgs;
    queueMessage(fd, modeMsg);
    broadcastToChannel(targetName, modeMsg, fd);
}

void    Server::handleCommand(int fd)
{
    Client      *client;
    std::string cmd;

    client = getClient(fd);
    if (!client || _message.command.empty())
        return ;
    cmd = _message.command;
    if (cmd == "CAP")
        return ;
    if (cmd == "PASS")
        cmdPass(fd);
    else if (cmd == "NICK")
        cmdNick(fd);
    else if (cmd == "USER")
        cmdUser(fd);
    else if (cmd == "PING")
        cmdPing(fd);
    else if (cmd == "QUIT")
        cmdQuit(fd);
    else if (cmd == "PONG")
        return ;
    else if (!client->isRegistered())
        numericReply(fd, "451", "", "You have not registered");
    else if (cmd == "JOIN")
        cmdJoin(fd);
    else if (cmd == "PART")
        cmdPart(fd);
    else if (cmd == "PRIVMSG")
        cmdPrivmsg(fd);
    else if (cmd == "KICK")
        cmdKick(fd);
    else if (cmd == "INVITE")
        cmdInvite(fd);
    else if (cmd == "TOPIC")
        cmdTopic(fd);
    else if (cmd == "MODE")
        cmdMode(fd);
    else
        numericReply(fd, "421", cmd, "Unknown command");
}
