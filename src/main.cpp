/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 09:57:04 by lchapard          #+#    #+#             */
/*   Updated: 2025/04/08 11:23:17 by vdomasch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

#include "Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Please execute as ./Webserv \"config_file_name\"!" << std::endl;
		return (1);
	}
	std::string config_variables[7] = {"listen", "host", "server_name", "error_page", "client_max_body_size", "root", "index"};
	HTTPConfig http_config;
	http_config.set_filename(argv[1]);
	if (http_config.parse_http())
		return (1);

	try {
		Server server;

		server.run_server(http_config);
	}
	catch (...)
	{
		std::cerr << "Error: Server creation failed." << std::endl;
		return (1);
	}
	return (0);
}
