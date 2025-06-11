#ifndef HTTPCONFIG_HPP
# define HTTPCONFIG_HPP

# include "ServerConfig.hpp"



class HTTPConfig
{
	public:
		HTTPConfig();
		~HTTPConfig();

		bool parse_http();

		void set_filename(std::string filename);

		size_t									get_client_max_body_size();
		std::map<std::string, std::string>&		get_http_map();
		std::map<std::string, ServerConfig>&	get_server_list();

		void DEBUG_HTTP_show();


	private:
		std::map<std::string, std::string> _map_http;
		std::map<std::string, ServerConfig> _server_list;

		std::string _filename;
		bool		_is_http;
		bool		_is_server;
		bool		_is_location;

		bool	http_check_bracket(std::istringstream &iss, std::string key);
		bool	set_http_values(std::istringstream &iss, std::string key);
		bool	is_http(std::string key);
		bool	is_http_variable(std::string key);
		bool	is_server(std::string key);
		bool	is_location(std::string key);
		bool	is_location_valid(std::istringstream &iss);
		bool	are_mandatory_directives_missing(ServerConfig &server_temp);
};

#endif