#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "LocationConfig.hpp"

template <typename T, typename Y> void convert(const T& from, Y& to);

class ServerConfig
{
	public:
		ServerConfig();
		ServerConfig(const ServerConfig&, std::string port);
		~ServerConfig();

		bool			parse_server(std::istringstream &iss, std::string key);
		bool			select_current_location(std::istringstream &iss, std::string key, std::string current_location_path);
		bool			set_server_values(std::istringstream &iss, std::string key);
		bool			add_location(std::string key);
		bool			duplicate_server(std::map<std::string, ServerConfig> &server_list);

		LocationConfig getMatchingLocation(const std::string& target) const;

		size_t									get_client_max_body_size();
		unsigned int							get_port_number();
		std::string								get_string_port_number();
		std::string								get_server_name();
		std::map<std::string, LocationConfig>	get_location_list();
		std::map<std::string, std::string>		get_map_server();

		std::string		DEBUG_test();

	private:
		std::map<std::string, std::string>		_map_server;
		std::vector<std::string>				_listen_ports;
		std::map<std::string, LocationConfig>	_location_list;
		std::string								_server_directives[16];
	
		bool	is_server_variable(std::string key);
		bool	handle_listen(std::istringstream &iss, std::map<std::string, std::string> &_map_server);
		bool	handle_host(std::istringstream &iss, std::map<std::string, std::string> &_map_server);

};

#endif