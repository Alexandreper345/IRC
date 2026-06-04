#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <map>


class Server
{
private:
	/* data */
public:
	Server(int port, const std::string& host);
	~Server();
};

#endif