#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "LocationConfig.hpp"

class ServerConfig
{
	public:
		ServerConfig();
		ServerConfig(const ServerConfig&, std::string port);
		~ServerConfig();

		bool			parse_server(std::istringstream &iss, std::string key);
		bool			select_current_location(std::istringstream &iss, std::string key);
		bool			set_server_values(std::istringstream &iss, std::string key);
		void			add_location(std::string key);
		bool			duplicate_server(std::map<std::string, ServerConfig> &server_list);
		unsigned int	get_port_number();
		std::string		get_string_port_number();
		std::string		DEBUG_test();
		std::string		get_server_name();
		std::map<std::string, std::string>	get_map_server();
	private:
		std::map<std::string, std::string> _map_server;
		std::vector<std::string> _listen_ports;
		std::map<std::string, LocationConfig> _location_list;
		std::string _server_directives[16];
	
		bool	is_server_variable(std::string key);
		bool	handle_listen(std::istringstream &iss, std::map<std::string, std::string> &_map_server);
		bool	handle_host(std::istringstream &iss, std::map<std::string, std::string> &_map_server);

};

#endif