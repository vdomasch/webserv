#ifndef SERVER_HPP
# define SERVER_HPP

# include "webserv.hpp"
# include <iostream>
# include <sys/select.h>
# include <signal.h>
# include <arpa/inet.h>
# include <set>


class Server
{
	public:
		Server();
		~Server();
		
		void		run_server(HTTPConfig &http_config);

		//int					get_client_max_body_size();
		std::map<int, int>	get_port_to_socket_map() const;
		std::map<int, int> 	get_socket_to_port_map() const;
		
	private:
		t_fd_data		_socket_data;	// to keep track of all active sockets
		//HttpRequest		_req;			// to parse the request

		std::set<std::string>																_ip_port_bound;
		std::map<int, int>																	_port_to_socket_map;
		std::map<int, int>																	_socket_to_port_map;
		std::map<int , HttpRequest>															_socket_states;

		std::map<std::string, void(*)(HTTPConfig& , HttpRequest& , std::map<std::string, ServerConfig>&, t_fd_data &, std::string )>	_method_map;

		int			initialize_server(ServerConfig &server, sockaddr_in &servaddr);
		void		running_loop(HTTPConfig &http_config, sockaddr_in &servaddr);

		void		handle_new_connection(int fd, sockaddr_in &servaddr);
		void		handle_client_request(HTTPConfig &http_config, int fd);

		bool		is_server_socket(int fd);
		int			close_msg(int fd, const std::string &message, int err, int return_code);
		void		update_max_fd(int fd);
		void		shutdown_all_sockets();
		std::string	get_server_name(int fd);

};

#endif