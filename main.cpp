#include <iostream>
#include "src/Server.hpp"

static int  stringToInt(char *str) {
    char    *end;
    long    n;
    int     value;
    
    n = std::strtol(str, &end, 10);
    if (*str == '\0' || *end != '\0')
        return (-1);
    if (n <= 0 || n > INT_MAX)
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
    const std::string password(argv[2]);

    Server server(port, password);
}