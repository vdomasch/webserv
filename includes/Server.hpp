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

		std::map<int, int>	_port_socket_map;

		int		initialize_server(ServerConfig &server);
		
};

#endif