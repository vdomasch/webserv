/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bhumeau <bhumeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 09:57:04 by lchapard          #+#    #+#             */
/*   Updated: 2025/03/12 13:41:32 by bhumeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

#include <fstream>

int	initialize_socket(sockaddr_in *servaddr, t_fd_data *socket_data)
{
	int	server_fd;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		perror("cannot create socket"); 
		return (-1); 
	}

	bzero(servaddr, sizeof(*servaddr));
	bzero(socket_data, sizeof(*socket_data));

	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(SERV_PORT);
	//servaddr->sin_addr.s_addr = htonl(INADDR_ANY); // bind to all ports
	//OR 
	servaddr->sin_addr.s_addr = inet_addr("127.0.0.1"); // if to localhost only

	if (bind(server_fd, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0)
	{
		perror("cannot bind to socket"); 
		return (-1); 
	}
	if (listen(server_fd, 10) < 0)
	{
		perror("Failed to lizlibsten ! ");
		return (-1);
	}
	return (server_fd);
}

int accept_connexion(int server_fd, sockaddr_in *servaddr)
{
	int	my_socket;
	int	addr_len = sizeof(servaddr);

	if ((my_socket = accept(server_fd, (struct sockaddr *)servaddr, (socklen_t*)&addr_len))<0)
	{
		perror("In accept");
		return(0); //exit ?
	}
	return (my_socket);
}

std::string find_in_config_file(std::string content, std::string variable_name)
{
	std::size_t i = 0;
	i = content.find(variable_name);
	if (i != content.rfind(variable_name))
		return ("-1");
	if (i == std::string::npos)
		return ("unset");
	i += variable_name.length();
	std::string res;
	while (content[i] && content[i] != ';')
	{
		res.push_back(content.at(i));
		i++;
	}
	return (res);
}

bool	copy_variable_values(std::string content, std::string config_variables[], std::string	struct_values[])
{
	for (int i = 0; i < 7; i++)
	{
		struct_values[i] = find_in_config_file(content, config_variables[i]);
		std::cout << config_variables[i] << " = " << struct_values[i] << std::endl;
		if (struct_values[i] == "unset")
		{
			std::cout << config_variables[i] << " not set in config file!" << std::endl;
			return (true);
		}
		if (struct_values[i] == "-1")
		{
			std::cout << config_variables[i] << " set more than once in config file!" << std::endl;
			return (true);
		}
	}
	return false;
}

std::string	copy_content(std::string filename)
{
	std::string content;
	std::string line;
	std::ifstream infile(filename.c_str());
	if (!infile.is_open())
	{
		std::cout << "Error, failed to open filename!" << std::endl;
		return ("NULL");
	}
	while(infile)
	{
		std::getline(infile, line);
		if(!infile)
			break;
		content += line + '\n';
		line.clear();
	}
	if (content.empty())
	{
		std::cout << "Config file is empty!" << std::endl;
		return ("NULL");
	}
	return (content);
}

bool	parse_config(std::string filename, std::string config_variables[])
{
	std::string content = copy_content(filename);
	if (content == "NULL")
		return true;
	std::string	struct_values[7] = {"unset", "unset", "unset", "unset", "unset", "unset", "unset"};
	if (!copy_variable_values(content, config_variables, struct_values))
		return true;
	return false;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Please execute as ./Webserv \"config_file_name\"!" << std::endl;
		return (1);
	}
	std::string config_variables[7] = {"listen", "host", "server_name", "error_page", "client_max_body_size", "root", "index"};
	if (parse_config(argv[1], config_variables))
		return (1);	
	int my_socket;
	int	server_fd;
	struct sockaddr_in servaddr;
	t_fd_data s_data; // to set select

	if (argc != 2)
	{
		std::cout << "Wrong number of arguments ! " << std::endl;
		return (0);
	}	

	server_fd = initialize_socket(&servaddr, &s_data);
	if (server_fd < 0)
	{
		perror("cannot bind to socket");
		return (0);
	}
	FD_ZERO(&s_data.saved_sockets);
	FD_SET(server_fd, &s_data.saved_sockets);
	while(42)
	{	
		printf("\n\033[31m++ Waiting for new connection ++\033[0m\n\n");
		s_data.ready_sockets = s_data.saved_sockets;
		if (select(FD_SETSIZE, &s_data.ready_sockets, NULL, NULL, NULL) < 0)
		{
			perror("Select failed ! ");
			return (0);
		}

		for (int i = 0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &s_data.ready_sockets))
			{
				if (i == server_fd)
				{
					my_socket = accept_connexion(server_fd, &servaddr);
					FD_SET(my_socket, &s_data.saved_sockets); //add new connection to current set
				}
				else
					printf("Nuhhh uhhh\n");
			}
		}

				
		char buffer[BUFFER_SIZE] = {0};
		const char *mess = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
		if (read(my_socket , buffer, BUFFER_SIZE) < 0)
		{
			perror("Failed to read ! ");
			return (0);
		}
		printf("------------------------\n");
		printf("%s\n",buffer );
		printf("------------------------\n");
		write(my_socket , mess , strlen(mess));
		std::cout << "message sent from server !\n" << std::endl;
		close(my_socket);
	}
	return (0);
}
