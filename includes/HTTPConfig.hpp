#ifndef HTTPCONFIG_HPP
# define HTTPCONFIG_HPP

# include "ServerConfig.hpp"
# include <iostream>
# include <vector>
# include <map>
# include <cstdlib>

class HTTPConfig
{
	public:
		HTTPConfig();
		~HTTPConfig();

		bool parse_http();
		void set_filename(std::string filename);
	private:
		std::map<std::string, std::string> _map_http;

		//std::string	_error_page;
		//std::string	_client_max_body_size;
		std::string _filename;
		std::vector<ServerConfig> _server_list;

		bool		_is_http;
		bool		_is_server;
		bool		_is_location;

		bool set_http_values(std::istringstream &iss, std::string key);
		bool is_http(std::string key);
		bool is_http_variable(std::string key);
		bool is_server(std::string key);
		bool is_location(std::string key);

		std::string clean_semicolon(std::string text);

};

#endif