#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "parsing_utils.hpp"

template <typename T, typename Y> void convert(const T& from, Y& to);

class LocationConfig
{
	public:
		LocationConfig();
		//LocationConfig(const LocationConfig&);
		~LocationConfig();

		void	set_path(std::string key);
		bool	parse_location(std::istringstream &iss, std::string key);

		int									get_client_max_body_size();
		std::map<std::string, std::string>	get_map_location();

	private:
		std::map<std::string, std::string>	_map_location;
		std::string							_location_directives[11];

		bool	is_location_variable(std::string key);

};

#endif