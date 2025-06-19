#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "parsing_utils.hpp"

template <typename Y, typename T> Y convert(const T& from);

class LocationConfig
{
	public:
		LocationConfig();
		LocationConfig(std::string path);
		~LocationConfig();

		bool	parse_location(std::istringstream &iss, std::string key);
		bool	check_cgi();
		std::string	DEBUG_test();

		std::string							get_root();
		std::string							get_path();
		std::string							get_index();
		size_t								get_client_max_body_size();
		bool								get_autoindex(bool autoindex);
		std::map<std::string, std::string>&	get_map_location();

	private:
		std::map<std::string, std::string>	_map_location;
		
		std::string _location_directives[10];
		bool handle_cgi_path(std::istringstream &iss, std::map<std::string, std::string> &_current_map);
		bool handle_cgi_ext(std::istringstream &iss, std::map<std::string, std::string> &_current_map);
		bool	is_location_variable(std::string key);

};

#endif