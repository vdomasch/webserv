/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bhumeau <bhumeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 10:01:15 by lchapard          #+#    #+#             */
/*   Updated: 2025/03/14 17:58:23 by bhumeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <iostream>// std::cout
#include <sys/socket.h> // socket
#include <strings.h> // bzero
#include <netinet/in.h> // struct addr
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // read & write
#include <string.h> // strlen
#include <vector> // vector

#define SERV_PORT 8080
#define BUFFER_SIZE 200

struct s_server
{
	std::string listen;
	std::string host;
	std::string server_name;
	std::string error_page;
	std::string client_max_body_size;
	std::string root;
	std::string index;
};
typedef struct s_server		t_server;

struct s_config
{
	// html elements
	std::vector<s_server> server;
	// vector of location (inside vector server?)	
};
typedef struct s_config		t_config;

struct s_fd_data
{
	fd_set  ready_sockets;
	fd_set  saved_sockets;	// current sockets
};
typedef struct s_fd_data	t_fd_data;
