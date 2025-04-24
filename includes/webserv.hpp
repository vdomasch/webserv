#ifndef WEBSSERV_HPP
# define WEBSSERV_HPP

# include "HTTPConfig.hpp"

# define SERV_PORT 8080
# define BUFFER_SIZE 2000

# include "HttpRequest.hpp"

typedef struct s_fd_data
{
	fd_set  ready_sockets;
	fd_set  saved_sockets;
	int		max_fd;
}	t_fd_data;

void	get_request(HttpRequest &, std::map<std::string, ServerConfig> &);
void	post_request(HttpRequest &, std::map<std::string, ServerConfig> &);
void	delete_request(HttpRequest &, std::map<std::string, ServerConfig> &);



std::string create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection);

std::string tostr(const int value);

#endif