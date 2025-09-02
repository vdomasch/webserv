
#ifndef METHODS_UTILS_HPP
# define METHODS_UTILS_HPP

# include "webserv.hpp"

std::string	handleCGI(HttpRequest &req, t_fd_data &d, int *errcode);

int	check_object_type(const std::string& path, int *errcode);

std::string	remove_prefix(std::string target, const std::string prefix);

void	build_response(HttpRequest &req, const std::string &status_code, const std::string &body, bool close_connection);

std::string	displayErrorPage(const std::string& code, HTTPConfig& http_config, HttpRequest& req, t_fd_data& fd_data);

std::string	find_error_page(const std::string& code, const std::string& location_name, const std::string& server_name, HTTPConfig& http);

bool	check_allowed_methods(ServerConfig &server, LocationConfig &location, const std::string &method);

std::string	validate_request_context(std::string &location_name, std::string &root, int &errcode, ServerConfig &server, const std::string &method);

#endif
