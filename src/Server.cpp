#include "Server.hpp"

Server::Server(int port, const std::string& password)
    : _port(port), _password(password) {}

Server::~Server(void) {}

void    Server::initServerSocket()
{
    _listening = socket(AF_INET, SOCK_STREAM, 0);
    if (!_listening)
    {
        std::cerr << "Could not create socket." << std::endl;
        exit (-1);
    }

    _hint.sin_family = AF_INET;
    _hint.sin_port = htons(1500);
    _hint.sin_addr.s_addr = htons(INADDR_ANY);
}
void    Server::acceptClient()
{

}
void    Server::bindSocket()
{
    int result;

    result = bind(_listening, reinterpret_cast<sockaddr *>(&_hint), sizeof(_hint));
    if (!result)
    {
        std::cerr << "Could not bind to port." << std::endl;
        exit (-2);
    }
}
void    Server::listenForConnections()
{
    int result;

    result = listen(_listening, SOMAXCONN);
    if (!result)
    {
        std::cerr << "Could not start listening." << std::endl;
        exit (-3);
    }
}
void    Server::receiveData(int fd)
{

}
void    Server::handlePoll()
{
    
}