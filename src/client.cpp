#include "webserv.hpp"
#include <arpa/inet.h>

int main()
{
	int sock = 0; long valread;
	struct sockaddr_in serv_addr = sockaddr_in();
	std::string hello = "This is what is send from the client";
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout << "\n Socket creation error " << std::endl;
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	    
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		std::cerr << "\nInvalid address/ Address not supported " << std::endl;
		return -1;
	}
	    
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cerr << "\nConnection Failed " << std::endl;
		return -1;
	}
	send(sock , hello.c_str() , hello.size(), 0 );
	std::cout << "Hello message sent" << std::endl;
	valread = read(sock , buffer, 1024);
	std::cout << buffer << std::endl;
	(void)valread;
	return 0;
}
