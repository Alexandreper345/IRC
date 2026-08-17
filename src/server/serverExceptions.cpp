#include "Server.hpp"

const char	*Server::ServerSocketError::what() const throw() {
    return "Server socket error.";
}

const char	*Server::SetNonBlockError::what() const throw() {
    return "Could not set socket to NONBLOCK.";
}

const char	*Server::BindPortError::what() const throw() {
    return "Could not bind port.";
}

const char	*Server::PollError::what() const throw() {
    return "Error running poll().";
}

const char	*Server::ListeningError::what() const throw() {
    return "Could not start listening.";
}