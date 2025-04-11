#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "parsing_utils.hpp"
# include "LocationConfig.hpp"

# include <iostream>
# include <fstream> //ifstream
# include <sstream> //
# include <map>
# include <vector>


class ServerConfig
{
	public:
		ServerConfig();
		//ServerConfig(const ServerConfig&);
		~ServerConfig();

		bool	parse_server(std::istringstream &iss, std::string key);
		void	show();
		bool	select_current_location(std::istringstream &iss, std::string key, int location_number);
		bool	set_server_values(std::istringstream &iss, std::string key);
		void	add_location(std::string key, int location_number);
	private:
		std::map<std::string, std::string> _map_server;
		std::vector<std::string> _listen_ports;
		std::vector<LocationConfig>_location_list;
		std::string _server_directives[23];

		bool	is_server_variable(std::string key);
};

#endif