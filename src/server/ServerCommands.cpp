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
    Client  *client;

    client = getClient(fd);
    if (!client)
        return (false);
    return (client->getChannels().find(channel) != client->getChannels().end());
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
    std::map<int, Client>::iterator it;

    for (it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->first != exceptFd && isInChannel(it->first, channel))
            queueMessage(it->first, message);
    }
}

void    Server::sendNames(int fd, const std::string& channel)
{
    Client                          *client;
    std::map<int, Client>::iterator it;
    std::string                     names;
    std::string                     nick;

    client = getClient(fd);
    if (!client)
        return ;
    nick = client->getNickname();
    for (it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (isInChannel(it->first, channel) && !it->second.getNickname().empty())
        {
            if (!names.empty())
                names += " ";
            names += it->second.getNickname();
        }
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
    Client                      *client;
    std::string                 reason;
    std::string                 msg;
    std::set<std::string>       chans;
    std::set<std::string>::const_iterator it;

    client = getClient(fd);
    if (!client)
        return ;
    reason = _message.params.empty() ? "Client Quit" : _message.params[0];
    msg = client->getPrefix() + " QUIT :" + reason;
    chans = client->getChannels();
    for (it = chans.begin(); it != chans.end(); ++it)
        broadcastToChannel(*it, msg, fd);
    removeClient(fd);
}

void    Server::cmdJoin(int fd)
{
    Client                      *client;
    std::vector<std::string>    channels;
    std::string                 joinMsg;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty() || _message.params[0].empty())
    {
        numericReply(fd, "461", "JOIN", "Not enough parameters");
        return ;
    }
    channels = splitComma(_message.params[0]);
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (!isValidChannel(channels[i]))
        {
            numericReply(fd, "403", channels[i], "No such channel");
            continue ;
        }
        if (isInChannel(fd, channels[i]))
            continue ;
        client->addChannel(channels[i]);
        joinMsg = client->getPrefix() + " JOIN :" + channels[i];
        queueMessage(fd, joinMsg);
        broadcastToChannel(channels[i], joinMsg, fd);
        numericReply(fd, "331", channels[i], "No topic is set");
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
        if (!isInChannel(fd, channels[i]))
        {
            numericReply(fd, "442", channels[i], "You're not on that channel");
            continue ;
        }
        partMsg = client->getPrefix() + " PART " + channels[i] + " :" + reason;
        queueMessage(fd, partMsg);
        broadcastToChannel(channels[i], partMsg, fd);
        client->removeChannel(channels[i]);
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
    Client                      *client;
    Client                      *target;
    std::string                 channel;
    std::string                 nick;
    std::string                 reason;
    std::string                 kickMsg;
    int                         targetFd;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.size() < 2)
    {
        numericReply(fd, "461", "KICK", "Not enough parameters");
        return ;
    }
    channel = _message.params[0];
    nick = _message.params[1];
    reason = (_message.params.size() > 2) ? _message.params[2] : client->getNickname();
    if (!isInChannel(fd, channel))
    {
        numericReply(fd, "442", channel, "You're not on that channel");
        return ;
    }
    targetFd = findClientByNick(nick);
    if (targetFd == -1)
    {
        numericReply(fd, "401", nick, "No such nick/channel");
        return ;
    }
    if (!isInChannel(targetFd, channel))
    {
        numericReply(fd, "441", nick + " " + channel, "They aren't on that channel");
        return ;
    }
    target = getClient(targetFd);
    kickMsg = client->getPrefix() + " KICK " + channel + " " + nick + " :" + reason;
    queueMessage(fd, kickMsg);
    broadcastToChannel(channel, kickMsg, fd);
    if (target)
        target->removeChannel(channel);
}

void    Server::cmdInvite(int fd)
{
    Client      *client;
    std::string nick;
    std::string channel;
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
    channel = _message.params[1];
    if (!isInChannel(fd, channel))
    {
        numericReply(fd, "442", channel, "You're not on that channel");
        return ;
    }
    targetFd = findClientByNick(nick);
    if (targetFd == -1)
    {
        numericReply(fd, "401", nick, "No such nick/channel");
        return ;
    }
    if (isInChannel(targetFd, channel))
    {
        numericReply(fd, "443", nick + " " + channel, "is already on channel");
        return ;
    }
    numericReply(fd, "341", nick + " " + channel, "");
    queueMessage(targetFd, client->getPrefix() + " INVITE " + nick + " :" + channel);
}

void    Server::cmdTopic(int fd)
{
    Client      *client;
    std::string channel;
    std::string topicMsg;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty())
    {
        numericReply(fd, "461", "TOPIC", "Not enough parameters");
        return ;
    }
    channel = _message.params[0];
    if (!isInChannel(fd, channel))
    {
        numericReply(fd, "442", channel, "You're not on that channel");
        return ;
    }
    if (_message.params.size() == 1)
    {
        numericReply(fd, "331", channel, "No topic is set");
        return ;
    }
    topicMsg = client->getPrefix() + " TOPIC " + channel + " :" + _message.params[1];
    queueMessage(fd, topicMsg);
    broadcastToChannel(channel, topicMsg, fd);
}

void    Server::cmdMode(int fd)
{
    Client      *client;
    std::string target;
    std::string modeMsg;

    client = getClient(fd);
    if (!client)
        return ;
    if (_message.params.empty())
    {
        numericReply(fd, "461", "MODE", "Not enough parameters");
        return ;
    }
    target = _message.params[0];
    if (target[0] != '#')
        return ;
    if (!isInChannel(fd, target))
    {
        numericReply(fd, "442", target, "You're not on that channel");
        return ;
    }
    if (_message.params.size() == 1)
    {
        queueMessage(fd, std::string(":") + SERVER_NAME + " 324 "
            + client->getNickname() + " " + target + " +");
        return ;
    }
    modeMsg = client->getPrefix() + " MODE " + target;
    for (size_t i = 1; i < _message.params.size(); i++)
        modeMsg += " " + _message.params[i];
    queueMessage(fd, modeMsg);
    broadcastToChannel(target, modeMsg, fd);
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
