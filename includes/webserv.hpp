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

void	get_request(HttpRequest &req);
void	post_request(HttpRequest &req);
void	delete_request(HttpRequest &req);

#endif