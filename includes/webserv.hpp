/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 10:01:15 by lchapard          #+#    #+#             */
/*   Updated: 2025/04/07 15:31:41 by vdomasch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSSERV_HPP
# define WEBSSERV_HPP

# include "HTTPConfig.hpp"
# include "ServerConfig.hpp"

# include <stdio.h>
# include <iostream>// std::cout
# include <sys/socket.h> // socket
# include <strings.h> // bzero
# include <netinet/in.h> // struct addr
# include <arpa/inet.h> // inet_addr
# include <unistd.h> // read & write
# include <string.h> // strlen

# define SERV_PORT 8080
# define BUFFER_SIZE 200

typedef struct s_fd_data
{
	fd_set  ready_sockets;
	fd_set  saved_sockets;	// current sockets
	int		max_fd;
}	t_fd_data;

bool is_keyword(std::string key, std::string pattern);


#endif