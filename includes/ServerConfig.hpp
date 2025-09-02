#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "LocationConfig.hpp"
# include <set>

template <typename Y, typename T> Y convert(const T& from);

class ServerConfig
{
	public:
		ServerConfig();
		ServerConfig(const ServerConfig&, std::string port);
		~ServerConfig();

		bool			parse_server(std::istringstream &iss, std::string key);
		bool			select_current_location_to_parse(std::istringstream &iss, std::string key, std::string current_location_path);
		bool			set_server_values(std::istringstream &iss, std::string key);
		bool			add_location(std::string key);
		bool			duplicate_server(std::map<std::string, ServerConfig> &server_list);
		bool			is_allow_methods_declared(std::map<std::string, std::string> &_current_map);
		void			set_get(std::map<std::string, std::string> &_current_map);


		size_t									get_client_max_body_size();
		bool									get_autoindex();
		unsigned int							get_uint_port_number();
		std::string								get_port_number();
		std::string								get_server_name();
		std::string								get_host_ip() const;
		std::string								get_root();
		std::map<std::string, LocationConfig>&	get_location_list();
		std::map<std::string, std::string>&		get_map_server();
		std::set<std::string>					get_authorized_paths() const;

		void			add_authorized_paths(const std::string& path);

		std::string		get_matching_location(const std::string& target, bool &autoindex);

		std::string		DEBUG_test();

	private:

		std::map<std::string, std::string>		_map_server;
		std::vector<std::string>				_listen_ports;
		std::map<std::string, std::string>		_ip_and_ports_association;
		std::map<std::string, LocationConfig>	_location_list;
		std::string								_server_directives[13];

		std::set<std::string>	_authorized_paths;

	
		bool	is_server_variable(std::string key);
		bool	handle_listen(std::istringstream &iss, std::map<std::string, std::string> &_map_server);
		bool	handle_host(std::string host_value);
		bool	handle_listen_ip_port(std::string &value);
		bool	handle_listen_port(std::string &value);
};

#endif