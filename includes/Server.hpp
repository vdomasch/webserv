#ifndef SERVER_HPP
# define SERVER_HPP

# include "webserv.hpp"
# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/select.h>
# include <signal.h>

typedef struct s_fd_data
{
	fd_set  ready_sockets;
	fd_set  saved_sockets; // current sockets
}	t_fd_data;

class Server
{
	public:
		Server();
		~Server();
		void		run_server();

	private:
		int			_server_fd;		// to store server socket
		int			_max_fd;		// to keep track of highest FD and not iterate on all 1024 FDs but only on all active ones
		int			_client_fd;
		bool		_running;
		t_fd_data	_socket_data;	// to keep track of all active sockets
		sockaddr_in	_servaddr;		// to store server address

		int		initialize_server();
		void	handle_new_connection();
		void	handle_existing_client();
};

#endif