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
		
		void		launch_server(HTTPConfig &http_config);

		std::map<int, int>	get_port_to_socket_map() const;
		std::map<int, int> 	get_socket_to_port_map() const;
		
	private:
		t_fd_data		_socket_data;

		std::set<std::string>																_ip_port_bound;
		std::map<int, int>																	_port_to_socket_map;
		std::map<int, int>																	_socket_to_port_map;
		std::map<int , HttpRequest>															_socket_states;

		std::map<std::string, void(*)(HTTPConfig& , HttpRequest& , t_fd_data & )>	_method_map;

		int			initialize_server(ServerConfig &server, sockaddr_in &servaddr);
		void		running_loop(HTTPConfig &http_config, sockaddr_in &servaddr);

		void		handle_new_connection(int fd, sockaddr_in &servaddr);
		void		handle_client_request(HTTPConfig &http_config, int fd);

		int			reading_data(int fd);
		bool		is_server_socket(int fd);
		std::string	get_server_name(int fd);
		void		get_ip_port(int fd);

		bool		is_conflicting_binding(const std::string& ip, std::string port, const std::set<std::string>& already_bound);
		bool		client_body_size_too_large(HttpRequest &request, HTTPConfig &http_config);

		void		update_max_fd(int fd);
		int			close_msg(int fd, const std::string &message, int err, int return_code);
		void		shutdown_all_sockets();
		void		clean_sockets();

};

#endif