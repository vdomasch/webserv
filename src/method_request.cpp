#include "webserv.hpp"

void	get_request(HttpRequest &req)
{
	std::cout << "GET request received" << std::endl;
	std::cout << req << std::endl;
}

void	post_request(HttpRequest &req)
{
	std::cout << "POST request received" << std::endl;
	std::cout << req << std::endl;
}

void	delete_request(HttpRequest &req)
{
	std::cout << "DELETE request received" << std::endl;
	std::cout << req << std::endl;
}