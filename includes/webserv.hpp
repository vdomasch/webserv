/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 10:01:15 by lchapard          #+#    #+#             */
/*   Updated: 2025/03/17 11:59:58 by vdomasch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <stdio.h>
# include <iostream>//std::cout
# include <sys/socket.h> //socket
# include <strings.h> //bzero
# include <netinet/in.h> // struct addr
# include <arpa/inet.h> // inet_addr
# include <unistd.h> //read & write
# include <string.h> // strlen
# include <fstream> // std::ofstream

# include "recieve_msg.hpp"

# define SERV_PORT 8080
# define BUFFER_SIZE 1024

void	parse_request(t_browser_request request, char *buffer, int client_fd);

#endif