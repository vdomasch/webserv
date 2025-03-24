#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <iostream>
# include <fstream> //ifstream
# include <sstream> //

class ServerConfig
{
	public:
		ServerConfig();
		//ServerConfig(const ServerConfig&);
		~ServerConfig();

		bool		server_config(std::string filename/*std::istringstream iss*/);
		private:
		std::string	_listen;
		std::string	_host;
		std::string	_server_name;
		std::string	_error_page;
		std::string	_client_max_body_size;
		std::string	_root;
		std::string	_index;
		std::string	_content;
		std::string	_config_variables[7];
		std::string	*_congif_values[7];

		std::string	find_in_config_file(std::string variable_name);
		bool		copy_variable_values();
		std::string	copy_content(std::string filename);

};

#endif