/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   html_request.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 09:47:18 by vdomasch          #+#    #+#             */
/*   Updated: 2025/03/19 13:33:12 by vdomasch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <sstream>

void	parse_request(t_browser_request request, char *buffer, int client_fd)
{
	
	(void)client_fd;
	if (buffer == NULL || request.bytes_read == 0)
		return ;

	request.done = 0;

	std::string str(buffer);	
	std::istringstream iss(str);
	std::getline(iss, request.method, ' ');
	std::getline(iss, request.location, ' ');
	

	std::string compare[] = {"Host", "Connection", "Accept"};
	
	while (request.done < 3)
	{
		//std::cout << "Parsing: " << compare[request.done] << std::endl;
		std::string line;
		std::getline(iss, line);
		std::size_t found = line.find(compare[request.done]);
		if (found != std::string::npos)
		{
			std::string value = line.substr(found + compare[request.done].length() + 2);
			switch (request.done)
			{
				case 0:
					request.host = value;
					break;
				case 1:
					request.connection = value;
					break; 	
				case 2:
					request.accept = value;
					break;
			}
			request.done++;
		}
	}

	std::cout << std::endl << "Request parsed: " << std::endl << std::endl;
	std::cout << "Bytes read: " << request.bytes_read << std::endl;
	std::cout << "Method: " << request.method << std::endl;
	std::cout << "Location: " << request.location << std::endl;
	std::cout << "Host: " << request.host << std::endl;
	std::cout << "Connection: " << request.connection << std::endl;
	std::cout << "Accept: " << request.accept << std::endl;
}