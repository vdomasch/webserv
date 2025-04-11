#ifndef RECIEVE_MSG_HPP
# define RECIEVE_MSG_HPP
 
#include <iostream>
#include <string>

typedef struct s_browser_request
{
	size_t bytes_read;
	std::string method;
	std::string location;
	std::string host;
	std::string connection;
	std::string accept;
	int done;
}	t_browser_request;

typedef struct s_server_response
{
	size_t bytes_read;
	std::string status;
	std::string status_message;
	std::string content_length;
	std::string content_type;
	std::string connection;
	std::string body;
	int done;
}	t_server_response;

#endif