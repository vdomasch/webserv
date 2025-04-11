#ifndef SERVER_HPP
# define SERVER_HPP

# include "webserv.hpp"
# include <iostream>
# include <sys/select.h>
# include <signal.h>

class Server
{
	public:
		Server();
		~Server();
		void		run_server(HTTPConfig &http_config);
		
	private:
		int			_server_fd;		// to store server socket
		int			_client_fd;
		t_fd_data	_socket_data;	// to keep track of all active sockets
		sockaddr_in	_servaddr;		// to store server address

		int		initialize_server(HTTPConfig &http_config);
		//void	handle_new_connection();
		//void	handle_existing_client();
};

#endif