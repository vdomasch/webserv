#ifndef PARSING_UTILS_HPP
# define PARSING_UTILS_HPP

# include <iostream>
# include <cstdlib>
# include <map>

class ServerConfig;

std::string	clean_semicolon(std::string text);
bool		is_keyword(std::string key, std::string pattern);
bool		is_error_page_code(std::string code);
bool		is_server_name_already_used(std::map<std::string, ServerConfig> &server_list, ServerConfig &server_temp);

#endif