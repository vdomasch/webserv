#ifndef WEBSSERV_HPP
# define WEBSSERV_HPP

# include "HTTPConfig.hpp"

# define SERV_PORT 8080
# define BUFFER_SIZE 200

typedef struct s_fd_data
{
	fd_set  ready_sockets;
	fd_set  saved_sockets;	// current sockets
}	t_fd_data;

#endif