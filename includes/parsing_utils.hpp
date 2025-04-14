/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bhumeau <bhumeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 10:01:15 by lchapard          #+#    #+#             */
/*   Updated: 2025/04/14 10:51:56 by bhumeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_UTILS_HPP
# define PARSING_UTILS_HPP

# include <iostream>
# include <cstdlib>
# include <map>

class ServerConfig;

bool		is_valid_to_clean_semicolon(std::string key);
std::string	clean_semicolon(std::string text);
bool		is_keyword(std::string key, std::string pattern);
bool		is_error_page_code(std::string code);
bool		is_server_name_already_used(std::map<std::string, ServerConfig> &server_list, ServerConfig &server_temp);

#endif