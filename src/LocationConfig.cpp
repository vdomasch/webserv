#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
{
	_location_directives[0] = "error_page";
	_location_directives[1] = "client_max_body_size";
	_location_directives[2] = "root";
	_location_directives[3] = "index";
	_location_directives[4] = "alias";
	_location_directives[5] = "autoindex";
	_location_directives[6] = "allow_methods";
	_location_directives[7] = "cgi_path";
	_location_directives[8] = "cgi_ext";
	_location_directives[9] = "return";
	_location_directives[10] = "rewrite";
}

//LocationConfig::LocationConfig(const LocationConfig& param) {}

LocationConfig::~LocationConfig() {}

void	LocationConfig::set_path(std::string key)
{
	_map_location["path"] = key;
}

std::map<std::string, std::string> LocationConfig::get_map_location()
{
	return _map_location;
}

bool	LocationConfig::parse_location(std::istringstream &iss, std::string key)
{
	if (!is_location_variable(key))
	{
		std::cerr << "Error: Invalid keyword '" << key << "'!" << std::endl;
		return 1;
	}
	else if (key == "error_page")
	{
		if (handle_error_page(iss, _map_location))
			return 1;
	}
	else if (key == "allow_methods")
	{
		if (handle_allow_methods(iss, _map_location))
			return 1;
	}
	else if (key == "autoindex")
	{
		if (handle_autoindex(iss, _map_location))
			return 1;
	}
	else
	{
		if (!_map_location[key].empty())
		{
			std::cerr << "Error: Keyword " << key << " already set!" << std::endl;
			return 1;
		}
		std::string value;
		iss >> value;
		if (!is_valid_to_clean_semicolon(value))
			return 1;
		value = clean_semicolon(value);
		_map_location[key] = value;
		if (!iss.eof())
		{
			std::cerr << "Error: There are values after ';' for keyword: " << key << "!" << std::endl;
			return 1;
		}
	}
	return 0;
}

bool	LocationConfig::is_location_variable(std::string key)
{
	for (int i = 0; i < 11; i++)
		if (key == _location_directives[i])
			return true;
	return false;
}
