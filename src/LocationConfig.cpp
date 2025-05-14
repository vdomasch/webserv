#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
{
	_location_directives[0] = "error_page";
	_location_directives[1] = "client_max_body_size";
	_location_directives[2] = "root";
	_location_directives[3] = "index";
	_location_directives[4] = "rewrite";
	_location_directives[5] = "autoindex";
	_location_directives[6] = "allow_methods";
	_location_directives[7] = "return";
}

//LocationConfig::LocationConfig(const LocationConfig& param) {}

LocationConfig::~LocationConfig() {}

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
	else if (key == "index")
	{
		if (handle_index(iss, _map_location))
			return 1;
	}
	else if (key == "root")
	{
		if (handle_root(iss, _map_location))
			return 1;
	}
	else
	{
		if (_map_location.count(key))
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
	if (_map_location.count("cgi"))
	{
		if (key == "cgi_path" || key == "cgi_ext")
			return true;
	}
	for (int i = 0; i < 11; i++)
		if (key == _location_directives[i])
			return true;
	return false;
}

std::string	LocationConfig::DEBUG_test()
{
	std::string str;
	for (std::map<std::string, std::string>::iterator it = _map_location.begin(); it != _map_location.end(); ++it)
	{
		str += it->first + ": " + it->second + "\n";
	}
	str += "\n";
	return str;
}

void	LocationConfig::set_cgi()
{
	_map_location["cgi"] = "on";
}
