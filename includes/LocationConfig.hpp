#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "parsing_utils.hpp"

class LocationConfig
{
	public:
		LocationConfig();
		~LocationConfig();

		bool	parse_location(std::istringstream &iss, std::string key);
		bool	check_cgi();
		std::string	DEBUG_test();

	private:
		std::map<std::string, std::string> _map_location;
		std::string _location_directives[10];

		bool	is_location_variable(std::string key);

};

#endif