#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "parsing_utils.hpp"

class LocationConfig
{
	public:
		LocationConfig();
		//LocationConfig(const LocationConfig&);
		~LocationConfig();

		bool	parse_location(std::istringstream &iss, std::string key);
		std::string	DEBUG_test();
		void	set_cgi();

	private:
		std::map<std::string, std::string> _map_location;
		std::string _location_directives[8];

		bool	is_location_variable(std::string key);

};

#endif