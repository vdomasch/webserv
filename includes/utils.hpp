#ifndef UTILS_HPP
# define UTILS_HPP

#include "webserv.hpp"

bool is_error(int errcode);

std::string	message_status(const std::string &status);

ServerConfig&	find_current_server(HTTPConfig& http_config, std::string &server_name);

std::string find_location_name_and_set_root(const std::string &target, ServerConfig &server, std::string &root, bool& autoindex);

void	build_response(HttpRequest &req, const std::string &status_code, const std::string &body, bool keep_alive_connection);

std::string	displayErrorPage(const std::string& code, HTTPConfig& http_config, HttpRequest& req, t_fd_data& fd_data);

std::string	find_error_page(const std::string& code, const std::string& location_name, const std::string& server_name, HTTPConfig& http);

#endif