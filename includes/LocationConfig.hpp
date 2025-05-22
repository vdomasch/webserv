#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "parsing_utils.hpp"

template <typename Y, typename T> Y convert(const T& from);

class LocationConfig
{
	public:
		LocationConfig();
		LocationConfig(std::string path);
		//LocationConfig(const LocationConfig&);
		~LocationConfig();

		bool	parse_location(std::istringstream &iss, std::string key);
		std::string	DEBUG_test();

		std::string							get_root();
		std::string							get_path();
		std::string							get_index();
		int									get_client_max_body_size();
		bool								get_autoindex();
		std::map<std::string, std::string>	get_map_location();

	private:
		std::map<std::string, std::string>	_map_location;
		
		std::string _location_directives[10];

		bool	is_location_variable(std::string key);

};

#endif