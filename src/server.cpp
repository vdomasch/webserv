/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bhumeau <bhumeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 09:57:04 by lchapard          #+#    #+#             */
/*   Updated: 2025/04/08 11:35:26 by bhumeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

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
	std::cout << "DEBUG HTTP parsed!" << std::endl;
	http_config.DEBUG_HTTP_show();
	return (0);
}
