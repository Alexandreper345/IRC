/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverExceptions.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 16:40:34 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/17 16:41:12 by anogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

const char	*Server::SignalSetupError::what() const throw() {
    return "Could not install signal handler.";
}