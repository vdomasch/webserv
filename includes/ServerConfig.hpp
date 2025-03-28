#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "parsing_utils.hpp"

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

		bool parse_server(std::istringstream &iss, std::string key);
		void show();
		bool	set_server_values(std::istringstream &iss, std::string key);

	private:
		std::map<std::string, std::string> _map_server;
		std::vector<std::string> _listen_ports;
		std::string _server_directives[23];

		bool	is_server_variable(std::string key);
};

#endif