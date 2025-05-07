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

		int					get_client_max_body_size();
		std::map<int, int>	get_port_to_socket_map() const;
		std::map<int, int> 	get_socket_to_port_map() const;
		
	private:
		t_fd_data		_socket_data;	// to keep track of all active sockets
		HttpRequest		_req;			// to parse the request

		std::map<int, int>								_port_to_socket_map;
		std::map<int, int>								_socket_to_port_map;
		std::map<int , t_requeste_state>				_socket_states;
		std::map<std::string, void(*)(HttpRequest&, std::map<std::string, ServerConfig>&)>	_method_map;

		int		initialize_server(ServerConfig &server, sockaddr_in &servaddr);
		void	update_max_fd(int fd);
		void	close_msg(int fd, const std::string &message, int err);
		void	running_loop(HTTPConfig &http_config, sockaddr_in &servaddr);
		bool	is_server_socket(int fd);
		void	shutdown_all_sockets();
		void	handle_new_connection(int fd, sockaddr_in &servaddr);
		void	handle_client_request(HTTPConfig &http_config, int fd, t_fd_data *socket_data);
};

#endif