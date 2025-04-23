#ifndef SERVER_HPP
# define SERVER_HPP

# include "webserv.hpp"
# include <iostream>
# include <sys/select.h>
# include <signal.h>
# include <arpa/inet.h>

class Server
{
	public:
		Server();
		~Server();
		void		run_server(HTTPConfig &http_config);
		
	private:
		t_fd_data	_socket_data;	// to keep track of all active sockets

		std::map<int, int>								_port_socket_map;
		std::map<std::string, void(*)(HttpRequest&)>	_method_map;

		int		initialize_server(ServerConfig &server, sockaddr_in &servaddr);
		void	update_max_fd(int fd);
		void	close_msg(int fd, const std::string &message, int err);
		void	running_loop(HTTPConfig &http_config, sockaddr_in &servaddr);
		bool	is_server_socket(int fd);
		void	shutdown_all_sockets();
};

#endif