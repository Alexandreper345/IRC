/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 17:33:06 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/17 18:49:39 by anogueir         ###   ########.fr       */
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

void Server::handleCommand(int fd)
{
    // NICK/USER/JOIN/KICK/INVITE/TOPIC/MODE/PART/QUIT/PRIVMSG/PASS/PING
    
    /* if (_message.command == "NICK")
    {
        _clients[fd].setNickname(_message.params[0]);
    }
    else if (_message.command == "USER")
    {
        _clients[fd].setUsername(_message.params[0]);
    }
    else if (_message.command == "JOIN")
    {
        _clients[fd].addChannel(_message.params[0]);
    }
    else if (_message.command == "KICK")
    {
        _clients[fd].kickClient(_message.params[0], _message.params[1]);
    }
    else if (_message.command == "INVITE")
    {
        _clients[fd].inviteClient(_message.params[0], _message.params[1]);
    }
    else if (_message.command == "TOPIC")
    {
        _clients[fd].setTopic(_message.params[0]);
    }
    else if (_message.command == "MODE")
    {
        _clients[fd].setMode(_message.params[0], _message.params[1]);
    }
    else if (_message.command == "PART")
    {
        _clients[fd].removeChannel(_message.params[0]);
    }
    else if (_message.command == "QUIT")
    {
        _clients[fd].quit();
    }
    else if (_message.command == "PRIVMSG")
    {
        _clients[fd].sendMessage(_message.params[0], _message.params[1]);
    }
    else if (_message.command == "PASS")
    {
        _clients[fd].setPassword(_message.params[0]);
    }
    else if (_message.command == "PING")
    {
        _clients[fd].sendPong(_message.params[0]);
    } */
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

    while (_clients[fd].getInBuffer().find("\r\n") != std::string::npos)
    {
        pos = _clients[fd].getInBuffer().find("\r\n");
        line = _clients[fd].getInBuffer().substr(0, pos);
        _clients[fd].getInBuffer().erase(0, pos + 2);
        parseLine(fd, line);
    }
}

void    Server::receiveData(int fd)
{
    char    recvBuffer[RECV_BUFFER_SIZE];
    int     recvSize;

    recvSize = recv(fd, recvBuffer, RECV_BUFFER_SIZE, 0);
    if (recvSize == 0)
    {
        removeClient(fd);
        return ;
    }
    _clients[fd].getInBuffer().append(recvBuffer, recvSize);
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

            if (revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                removeClient(fd);
                continue ;
            }
            if (revents & POLLIN)
                receiveData(fd);

            if (i < _fds.size() && _fds[i].fd == fd)
                i++;
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
