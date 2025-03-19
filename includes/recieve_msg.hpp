/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   recieve_msg.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/14 14:47:50 by vdomasch          #+#    #+#             */
/*   Updated: 2025/03/19 13:32:40 by vdomasch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RECIEVE_MSG_HPP
# define RECIEVE_MSG_HPP
 
#include <iostream>
#include <string>

typedef struct s_browser_request
{
	size_t bytes_read;
	std::string method;
	std::string location;
	std::string host;
	std::string connection;
	std::string accept;
	int done;
}	t_browser_request;

typedef struct s_server_response
{
	size_t bytes_read;
	std::string method;
	std::string location;
	std::string version;
	std::string host;
	std::string connection;
	std::string accept;
	std::string content_length;
	std::string content_type;
	std::string body;
	int done;
}	t_server_response;

#endif