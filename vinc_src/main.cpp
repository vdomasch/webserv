#include "webserv.hpp"
#include "../vinc_includes/recieve_msg.hpp"
#include "../vinc_includes/Server.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <map>

t_browser_request request;

std::string itostr(int number)
{
	std::stringstream ss;
	
	ss << number;
	return ss.str();
}

std::string	generate_http_response(std::string code, std::string code_desc, t_browser_request &request)
{
	std::string response = "HTTP/1.1 " + code + " " + code_desc + "\r\n";
	response += "Content-Length: " + itostr(230) + "\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Connection: " + request.connection + "\r\n";
	
	if (request.connection == "keep-alive")
		response += "Keep-Alive: timeout=10, max=100\r\n";
	
	response += "\r\n";
	response += "<!DOCTYPE html>\r\n"
				"<html lang=\"en\">\r\n"
				"\t<head>\r\n"
				"\t\t<meta charset=\"UTF-8\">\r\n"
				"\t\t<title>Webserv Page</title>\r\n"
				"\t</head>\r\n"
				"\t<body>\r\n"
				"\t\t<h1>Welcome to My Server</h1>\r\n"
				"\t\t<p>This is a simple HTML response from the server.</p>\r\n"
				"\t</body>\r\n"
				"</html>\r\n";
	return (response);
}