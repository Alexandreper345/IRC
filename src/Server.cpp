#include "Server.hpp"

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

void    Server::receiveData(int fd)
{
    char    recvBuffer[RECV_BUFFER_SIZE];
    int     recvSize;
    int     sentSize;

    recvSize = recv(fd, recvBuffer, RECV_BUFFER_SIZE, 0);
    if (recvSize <= 0)
    {
        removeClient(fd);
        return ;
    }
    _clients[fd].getInBuffer();
    sentSize = send(fd, recvBuffer, recvSize, 0);
    if (sentSize <= 0)
        removeClient(fd);
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
