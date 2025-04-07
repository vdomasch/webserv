/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 09:57:04 by vdomasch          #+#    #+#             */
/*   Updated: 2025/03/25 09:16:23 by vdomasch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "recieve_msg.hpp"
#include "Server.hpp"

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

int main(int argc, char **argv)
{
	(void)argc, (void)argv;
	//if (argc != 2 || argv == NULL)
	//	return (std::cout << "Wrong number of arguments! " << std::endl, 0);

	Server server;

	server.run_server();

}