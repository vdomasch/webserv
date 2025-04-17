/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bhumeau <bhumeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 10:01:15 by lchapard          #+#    #+#             */
/*   Updated: 2025/04/17 14:28:52 by bhumeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_UTILS_HPP
# define PARSING_UTILS_HPP

# include <iostream>
# include <cstdlib>
# include <map>
# include <vector>
# include <fstream>
# include <sstream>

class ServerConfig;

bool		is_valid_to_clean_semicolon(std::string key);
std::string	clean_semicolon(std::string text);
bool		is_keyword(std::string key, std::string pattern);
bool		is_error_page_code(std::string code);
bool		is_server_name_already_used(std::map<std::string, ServerConfig> &server_list, ServerConfig &server_temp);
bool		handle_error_page(std::istringstream &iss, std::map<std::string, std::string> &_map_http);
bool		handle_autoindex(std::istringstream &iss, std::map<std::string, std::string> &_map_server);
bool		handle_allow_methods(std::istringstream &iss, std::map<std::string, std::string> &_current_map);


#endif