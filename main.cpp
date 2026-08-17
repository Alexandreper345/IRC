/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anogueir <anogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 17:33:01 by anogueir          #+#    #+#             */
/*   Updated: 2026/08/17 18:24:06 by anogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cctype>
#include <cstdlib>
#include "include/Server.hpp"

static bool isPasswordValid(const std::string &password)
{
    if (password.empty())
        return (false);

    if (password.size() > 30)
        return (false);

    for (size_t i = 0; i < password.size(); ++i) {
        if (std::isspace(static_cast<unsigned char>(password[i])))
            return (false);
    }
    
    return (true);
}

static int  stringToInt(char *str) {
    char    *end;
    long    n;
    int     value;
    
    n = std::strtol(str, &end, 10);
    if (*str == '\0' || *end != '\0')
        return (-1);
    if (n <= 0 || n > 65535)
        return (-1);
    value = static_cast<int>(n);
    return (value);
}

static bool onlyDigits(char *str) {
    
    int i;
    
    if (str[0] == '\0')
        return (false);
    i = -1;
    while (str[++i] != '\0') {
        if (!isdigit(str[i]))
            return (false);
    }
    return (true);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Expected input: <port> <password>" << std::endl;
        return (1);
    }

    if (!onlyDigits(argv[1]))
    {
        std::cerr << "Invalid port number." << std::endl;
        return (2);
    }
        
    int port = stringToInt(argv[1]);
    if (port == -1)
    {
        std::cerr << "Invalid port number." << std::endl;
        return (2);
    }
    
    const std::string password(argv[2]);
    if (!isPasswordValid(password))
    {
        std::cerr << "Invalid password." << std::endl;
        return (3);
    }
    
    try
    {
        Server server(port, password);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
    
    return (0);
}