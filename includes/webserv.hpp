/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 10:01:15 by lchapard          #+#    #+#             */
/*   Updated: 2025/03/25 10:19:45 by vdomasch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <stdio.h>
# include <iostream>//std::cout
# include <sys/socket.h> //socket
# include <netinet/in.h> // struct addr
# include <arpa/inet.h> // inet_addr
# include <fstream> // std::ofstream
# include <sstream>
# include <cstring>


# include "recieve_msg.hpp"

# define SERV_PORT 8080
# define BUFFER_SIZE 1024

void		parse_request(t_browser_request & , char * );

std::string	itostr(int );
std::string	generate_http_response(std::string , std::string , t_browser_request &);


#endif