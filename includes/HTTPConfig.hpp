#ifndef HTTPCONFIG_HPP
# define HTTPCONFIG_HPP

# include "ServerConfig.hpp"
# include "parsing_utils.hpp"
# include <iostream>
# include <vector>
# include <map>

class HTTPConfig
{
	public:
		HTTPConfig();
		~HTTPConfig();

		bool parse_http();
		void set_filename(std::string filename);
		void DEBUG_HTTP_show();
	private:
		std::map<std::string, std::string> _map_http;
		std::map<unsigned int, ServerConfig> _server_list;

		std::string _filename;
		bool		_is_http;
		bool		_is_server;
		bool		_is_location;

		bool	set_http_values(std::istringstream &iss, std::string key);
		bool	is_http(std::string key);
		bool	is_http_variable(std::string key);
		bool	is_server(std::string key);
		bool	is_location(std::string key);
		bool	is_location_valid(std::istringstream &iss);
};

#endif