/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 17:33:06 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/17 20:27:10 by anogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Server.hpp"

static volatile sig_atomic_t g_stop = 0;

Server::Server(void) {}

Server::Server(const Server&) {}

Server	&Server::operator=(const Server&) { return (*this); }

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _socket(-1) {}

Server::~Server(void)
{
    for (size_t i = 0; i < _fds.size(); i++)
        close(_fds[i].fd);
}

void    Server::initServerSocket(void)
{
    int enable;

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        throw ServerSocketError();

    enable = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1)
        throw ServerSocketError();

    if (fcntl(_socket, F_SETFL, O_NONBLOCK) == -1)
        throw SetNonBlockError();

    memset(&_hint, 0, sizeof(_hint));
    _hint.sin_family = AF_INET;
    _hint.sin_port = htons(_port);
    _hint.sin_addr.s_addr = htonl(INADDR_ANY);

}

void    Server::bindSocket(void)
{
    int result;

    result = bind(_socket, reinterpret_cast<sockaddr *>(&_hint), sizeof(_hint));
    if (result == -1)
        throw BindPortError();
}

void    Server::listenForConnections(void)
{
    int result;

    result = listen(_socket, SOMAXCONN);
    if (result == -1)
        throw ListeningError();
}

void    Server::acceptClient(void)
{
    struct sockaddr_in  clientAddr;
    socklen_t           clientAddrlen;
    int                 clientFd;
    pollfd              p;

    clientAddrlen = sizeof(clientAddr);
    clientFd = accept(_socket,
                      reinterpret_cast<struct sockaddr *>(&clientAddr),
                      &clientAddrlen);
    if (clientFd == -1)
        return ;
    if (_fds.size() - 1 >= MAX_CLIENTS)
    {
        close(clientFd);
        return ;
    }
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(clientFd);
        throw SetNonBlockError();
    }
    p.fd = clientFd;
    p.events = POLLIN;
    p.revents = 0;
    _fds.push_back(p);
    _clients.insert(std::make_pair(clientFd, Client(clientFd)));
    _clients[clientFd].setHostname(inet_ntoa(clientAddr.sin_addr));
}

void    Server::removeClient(int fd)
{
    partClientFromChannels(fd, "");
    _clients.erase(fd);
    for (size_t i = 1; i < _fds.size(); )
    {
        if (_fds[i].fd == fd)
            _fds.erase(_fds.begin() + i);
        else
            i++;
    }
    close(fd);
}

void    Server::setPollOut(int fd, bool enable)
{
    for (size_t i = 1; i < _fds.size(); i++)
    {
        if (_fds[i].fd == fd)
        {
            if (enable)
                _fds[i].events = POLLIN | POLLOUT;
            else
                _fds[i].events = POLLIN;
            return ;
        }
    }
}

void    Server::sendData(int fd)
{
    Client  *client;
    ssize_t sent;

    client = getClient(fd);
    if (!client || client->getOutBuffer().empty())
        return ;
    sent = send(fd, client->getOutBuffer().c_str(), client->getOutBuffer().size(), 0);
    if (sent < 0)
    {
        setPollOut(fd, true);
        return ;
    }
    if (sent == 0)
    {
        removeClient(fd);
        return ;
    }
    client->getOutBuffer().erase(0, static_cast<size_t>(sent));
    if (client->getOutBuffer().empty())
        setPollOut(fd, false);
}

void Server::parseLine(int fd, std::string line)
{
    _message.prefix.clear();
    _message.command.clear();
    _message.params.clear();

    if (line.empty())
        return;

    if (line[0] == ':')
    {
        std::string::size_type sp = line.find(' ');
        if (sp == std::string::npos)
            return;
        _message.prefix = line.substr(1, sp - 1);
        line.erase(0, sp + 1);
        while (!line.empty() && line[0] == ' ')
            line.erase(0, 1);
    }

    std::string::size_type pos = line.find(' ');
    if (pos == std::string::npos)
        _message.command = line;
    else
    {
        _message.command = line.substr(0, pos);
        line.erase(0, pos + 1);
    }

    for (size_t i = 0; i < _message.command.size(); ++i)
        _message.command[i] = static_cast<char>(
            std::toupper(static_cast<unsigned char>(_message.command[i])));

    if (pos != std::string::npos)
    {
        while (!line.empty() && line[0] == ' ')
            line.erase(0, 1);
        while (!line.empty())
        {
            if (line[0] == ':')
            {
                _message.params.push_back(line.substr(1));
                break;
            }
            pos = line.find(' ');
            if (pos == std::string::npos)
            {
                _message.params.push_back(line);
                break;
            }
            _message.params.push_back(line.substr(0, pos));
            line.erase(0, pos + 1);
            while (!line.empty() && line[0] == ' ')
                line.erase(0, 1);
        }
    }
    handleCommand(fd);
}

void    Server::extractLines(int fd)
{
    std::string line;
    size_t      pos;
    size_t      nlen;
    Client      *client;

    client = getClient(fd);
    while (client)
    {
        pos = client->getInBuffer().find("\r\n");
        nlen = 2;
        if (pos == std::string::npos)
        {
            pos = client->getInBuffer().find('\n');
            nlen = 1;
        }
        if (pos == std::string::npos)
            break ;
        line = client->getInBuffer().substr(0, pos);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        client->getInBuffer().erase(0, pos + nlen);
        if (line.size() <= MAX_MSG_SIZE)
            parseLine(fd, line);
        client = getClient(fd);
    }
}

void    Server::receiveData(int fd)
{
    char    recvBuffer[RECV_BUFFER_SIZE];
    ssize_t recvSize;
    Client  *client;

    recvSize = recv(fd, recvBuffer, RECV_BUFFER_SIZE, 0);
    if (recvSize == 0)
    {
        removeClient(fd);
        return ;
    }
    if (recvSize < 0)
        return ;
    client = getClient(fd);
    if (!client)
        return ;
    client->getInBuffer().append(recvBuffer, static_cast<size_t>(recvSize));
    if (client->getInBuffer().size() > MAX_BUFFER_SIZE)
    {
        removeClient(fd);
        return ;
    }
    extractLines(fd);
}

void    Server::handlePoll(void)
{
    pollfd  server;
    int     ret;

    server.fd = _socket;
    server.events = POLLIN;
    server.revents = 0;
    _fds.push_back(server);

    while (!g_stop)
    {
        ret = poll(_fds.data(), _fds.size(), -1);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue ;
            throw PollError();
        }

        if (_fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
            throw ServerSocketError();
        if (_fds[0].revents & POLLIN)
            acceptClient();

        for (size_t i = 1; i < _fds.size(); )
        {
            int     fd = _fds[i].fd;
            short   revents = _fds[i].revents;

            if (revents & POLLIN)
                receiveData(fd);
            if (i < _fds.size() && _fds[i].fd == fd)
            {
                Client  *client;

                client = getClient(fd);
                if (client && !client->getOutBuffer().empty()
                    && (revents & (POLLOUT | POLLIN)))
                    sendData(fd);
            }
            if (i < _fds.size() && _fds[i].fd == fd)
            {
                Client  *client;

                client = getClient(fd);
                if ((revents & (POLLHUP | POLLERR | POLLNVAL))
                    && (!client || client->getOutBuffer().empty()))
                {
                    removeClient(fd);
                    continue ;
                }
                i++;
            }
        }
    }
}

static void handleStop(int)
{
    g_stop = 1;
}

void    Server::setupSignals(void)
{
    struct sigaction sa;

    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handleStop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT,  &sa, NULL) == -1)
        throw SignalSetupError();
    if (sigaction(SIGTERM, &sa, NULL) == -1)
        throw SignalSetupError();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL) == -1)
        throw SignalSetupError();
}

void    Server::run(void)
{
    setupSignals();
    initServerSocket();
    bindSocket();
    listenForConnections();
    handlePoll();
}
