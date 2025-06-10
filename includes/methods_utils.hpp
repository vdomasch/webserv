
#ifndef METHODS_UTILS_HPP
# define METHODS_UTILS_HPP

# include "webserv.hpp"

std::string	handleCGI(t_fd_data &d, int *errcode);

int	check_object_type(std::string& path, int *errcode);

std::string	remove_prefix(std::string target, const std::string prefix);

std::string	normalize_path(const std::string &path);

void	build_response(HttpRequest &req, const std::string &status_code, const std::string &body, bool close_connection);

std::string	displayErrorPage(const std::string& code,
							const std::string& location_name,
							HTTPConfig& http_config,
							HttpRequest& req,
							t_fd_data& fd_data,
							const std::string& server_name);

std::string	find_error_page(const std::string& code, const std::string& location_name, const std::string& server_name, HTTPConfig& http);

std::string	create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection);

bool	check_allowed_methods(ServerConfig &server, LocationConfig &location, const std::string &method);

ServerConfig&	find_current_server(HTTPConfig& http_config, std::string &server_name);

std::string	message_status(const std::string &code);

#endif
