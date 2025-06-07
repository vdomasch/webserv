
#ifndef METHODS_UTILS_HPP
# define METHODS_UTILS_HPP

# include "webserv.hpp"

int	check_object_type(std::string& path, int *errcode);

std::string	normalize_path(const std::string &path);

void	build_response(HttpRequest &req, int status_code, const std::string &status_msg, const std::string &content_type, const std::string &body, bool close_connection);

std::string	displayErrorPage(const std::string code,
							const std::string message,
							const std::string& error_uri,
							HTTPConfig& http_config,
							HttpRequest& req,
							std::map<std::string, ServerConfig>& server_list,
							t_fd_data& fd_data,
							const std::string& server_name,
							bool is_error_request);

std::string	find_error_page(const std::string& code, LocationConfig* loc, ServerConfig& serv, HTTPConfig& http);

std::string	create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection);

bool	check_allowed_methods(ServerConfig &server, LocationConfig &location, const std::string &method);

#endif
