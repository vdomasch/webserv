#ifndef HTTPCONFIG_HPP
# define HTTPCONFIG_HPP

# include "ServerConfig.hpp"
# include <iostream>
# include <vector>

class HTTPConfig
{
	public:
		HTTPConfig();
		~HTTPConfig();

		bool parse_http();
		void set_filename(std::string filename);
	private:
		std::string	_error_page;
		std::string	_client_max_body_size;
		std::string _filename;
		std::vector<ServerConfig> _server_list;

		void set_http_values(std::istringstream &iss);
};

#endif