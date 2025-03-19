/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 09:57:04 by vdomasch          #+#    #+#             */
/*   Updated: 2025/03/18 13:43:04 by vdomasch         ###   ########.fr       */
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

int main(int argc, char **argv)
{
	(void)argc, (void)argv;
	//if (argc != 2 || argv == NULL)
	//	return (std::cout << "Wrong number of arguments! " << std::endl, 0);

	Server server;

	server.run_server();

}